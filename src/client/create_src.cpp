/**
 * @file create_src.hpp
 *
 * @copyright Copyright (C) 2014 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the srcml command-line client.
 *
 * The srcML Toolkit is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The srcML Toolkit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the srcml command-line client; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <create_src.hpp>
#include <srcml.h>
#include <src_output_libarchive.hpp>
#include <src_output_filesystem.hpp>
#include <src_prefix.hpp>
#include <curl_input_file.hpp>
#include <input_curl.hpp>
#include <input_file.hpp>
#include <input_archive.hpp>
#include <SRCMLStatus.hpp>

class srcMLReadArchive {
public:
    srcMLReadArchive(const srcml_input_src& input_source, const boost::optional<size_t>& revision) {
        arch = srcml_archive_create();
        if (!arch)
            throw srcMLReadArchiveError(0, input_source);

        int status = SRCML_STATUS_OK;

        if (revision) {
            status = srcml_archive_set_srcdiff_revision(arch, *revision);
            if (status != SRCML_STATUS_OK)
                throw status;
        }

        // may need to modify input source based on url and compressions
        srcml_input_src curinput = input_source;

        // urls
        if (curinput.protocol != "file" && curl_supported(curinput.protocol)) {
            srcml_input_src uninput = curinput;
            if (!input_curl(uninput))
                throw -1;

            curinput.fd = *uninput.fd;
        }

        // compressed files
        if (!curinput.compressions.empty() && curinput.archives.empty()) {
            srcml_input_src uninput = curinput;
            input_file(uninput);
            curinput.fd = *uninput.fd;
        }

        // archives (and possibly compressions)
        if (!curinput.archives.empty()) {
            curinput.fd = input_archive(curinput);
        }

        // open input source
        if (curinput.fd) {
            status = srcml_archive_read_open_fd(arch, *curinput.fd);
        } else {
            status = srcml_archive_read_open(arch, input_source);
        }
        if (status != SRCML_STATUS_OK) {
            SRCMLstatus(WARNING_MSG, "srcml: Unable to open srcml file " + src_prefix_resource(input_source.filename));
            throw status;
        }
    }

    operator srcml_archive*() { return arch; }

    ~srcMLReadArchive() {

        srcml_archive_close(arch);
        srcml_archive_free(arch);
    }

private:
    srcml_archive* arch;
};

// create srcml from the current request
void create_src(const srcml_request_t& srcml_request,
                const srcml_input_t& input_sources,
                const srcml_output_dest& destination) {

    try {

        if (option(SRCML_COMMAND_TO_DIRECTORY)) {

            // srcml->src extract all archives to the filesystem

            TraceLog log;

            for (const auto& input_source : input_sources) {
                srcMLReadArchive arch(input_source, srcml_request.revision);

                src_output_filesystem(arch, destination, log);
            }

        } else if (input_sources.size() == 1 && contains<int>(destination) &&
                   destination.compressions.empty() && destination.archives.empty()) {

            // srcml->src extract to stdout
            srcMLReadArchive arch(input_sources[0], srcml_request.revision);

            // move to the correct unit
            for (int i = 1; i < srcml_request.unit; ++i) {
                srcml_unit* unit = srcml_archive_read_unit_header(arch);
                if (!unit) {
                    SRCMLstatus(ERROR_MSG, "Requested unit %s out of range.", srcml_request.unit);
                    exit(1);
                }
                srcml_unit_free(unit);
            }

            int count = 0;
            while (1) {
                srcml_unit* unit = srcml_archive_read_unit_header(arch);
                if (srcml_request.unit && !unit) {
                    SRCMLstatus(ERROR_MSG, "Requested unit %s out of range.", srcml_request.unit);
                    exit(1);
                }
                if (!unit)
                    break;

                // set encoding for source output
                // NOTE: How this is done may change in the future
                if (srcml_request.src_encoding)
                    srcml_archive_set_src_encoding(arch, srcml_request.src_encoding->c_str());

                // null separator before every unit (except the first)
                if (count) {
                    write(1, "", 1);
                }

                // unaparse directly to the destintation
                srcml_unit_unparse_fd(unit, destination);

                srcml_unit_free(unit);

                // get out if only one unit
                if (srcml_request.unit)
                    break;

                ++count;
            }

        } else if (input_sources.size() == 1 && destination.compressions.empty() && destination.archives.empty()) {
            srcMLReadArchive arch(input_sources[0], srcml_request.revision);

            // move to the correct unit
            for (int i = 1; i < srcml_request.unit; ++i) {
                srcml_unit* unit = srcml_archive_read_unit_header(arch);
                if (!unit) {
                    SRCMLstatus(ERROR_MSG, "Requested unit %s out of range.", srcml_request.unit);
                    exit(1);
                }
                srcml_unit_free(unit);
            }

            srcml_unit* unit = srcml_archive_read_unit_header(arch);
            if (!unit) {
                SRCMLstatus(ERROR_MSG, "Requested unit %s out of range.", srcml_request.unit);
                exit(4);
            }

            // set encoding for source output
            // NOTE: How this is done may change in the future
            if (srcml_request.src_encoding)
                srcml_archive_set_src_encoding(arch, srcml_request.src_encoding->c_str());

            int status = srcml_unit_unparse_filename(unit, destination.c_str(), 0);
            if (status) {
                SRCMLstatus(ERROR_MSG, "srcml: unable to open output file " + destination.resource);
                exit(4);
            }

            srcml_unit_free(unit);

        } else {

            // srcml->src extract to libarchive file
            if (destination.archives.size() == 0) {
                SRCMLstatus(ERROR_MSG, "srcml: source output requires an archive format (tar, zip, etc.)");
                exit(1); //TODO: Need an error code
            }

            archive* ar = archive_write_new();

            // setup format
            for (const auto& ext : destination.archives)
                archive_write_set_format_by_extension(ar, ext.c_str());

            // setup compressions
            for (const auto& ext : destination.compressions)
                archive_write_set_compression_by_extension(ar, ext.c_str());

            int status = ARCHIVE_OK;
            if (contains<int>(destination)) {
                status = archive_write_open_fd(ar, destination);
            } else {
                status = archive_write_open_filename(ar, destination.resource.c_str());
            }
            if (status != ARCHIVE_OK) {
                SRCMLstatus(ERROR_MSG, std::to_string(status));
                exit(1);
            }

            // extract all the srcml archives to this libarchive
            for (const auto& input_source : input_sources) {

                srcMLReadArchive arch(input_source, srcml_request.revision);

                // extract this srcml archive to the source archive
                src_output_libarchive(arch, ar);
            }

            archive_write_close(ar);
#if ARCHIVE_VERSION_NUMBER >= 3000000
            archive_write_free(ar);
#else
            archive_write_finish(ar);
#endif
        }

    } catch (srcMLReadArchiveError e) {
        SRCMLstatus(ERROR_MSG, "Error " + std::to_string(e.status) + " with " + e.errmsg);
    } catch (...) {
        exit(1);
    }
}
