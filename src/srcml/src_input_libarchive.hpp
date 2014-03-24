/**
 * @file src_input_libarchive.hpp
 *
 * @copyright @copyright Copyright (C) 2014 SDML (www.srcML.org)
 *
 * This file is part of the srcML Toolkit.
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
 * along with the srcML Toolkit; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Source input from local files, stdin, and source archives for srcml parsing queue
 */

#ifndef SRC_INPUT_LIBARCHIVE_HPP
#define SRC_INPUT_LIBARCHIVE_HPP

#include <srcml.h>
#include <string>
#include <parse_queue.hpp>
#include <boost/optional.hpp>
#include <srcml_input_src.hpp>

// Extension that map to archive types
static const char* archive_extensions[] = {
#if ARCHIVE_VERSION_NUMBER >= 3000000
    ".7z",
#endif

    ".ar",

#if ARCHIVE_VERSION_NUMBER >= 3000000
    ".cab",
#endif

    ".cpio",
    ".iso",

#if ARCHIVE_VERSION_NUMBER >= 3000000
    ".lha",
    ".lzh",
#endif

    ".mtree",
    ".pax",

#if ARCHIVE_VERSION_NUMBER >= 3000000
    ".rar",
#endif

    ".shar",
    ".tar",
    ".taz",  // (archive w/ compression)
    ".tb2",  // (archive w/ compression)
    ".tbz",  // (archive w/ compression)
    ".tbz2", // (archive w/ compression)
    ".tgz",  // (archive w/ compression)
    ".tlz",  // (archive w/ compression)
    ".txz",  // (archive w/ compression)
    ".xar",
    ".zip",  // (archive w/ compression)
    0
};

// Extension that map to compression types
static const char* compression_extensions[] = {
    ".bz"
    ".bz2",
    ".gz",
    ".lz",
    ".lzma",
    ".xz",
    ".z",
    0
};

void src_input_libarchive(ParseQueue& queue,                  // srcml parsing queue
                          srcml_archive* srcml_arch,          // srcml archive to write to
                          const srcml_input_src& input,  // input source
                          const boost::optional<std::string>& language,        // specified language
                          const boost::optional<std::string>& option_filename,
                          const boost::optional<std::string>& option_directory,
                          const boost::optional<std::string>& option_version,
                          boost::optional<FILE*> fstdin = boost::optional<FILE*>()  // stdin (if already opened)
                          );

bool is_archive(const std::string& input_file_extension);
bool is_compressed(const std::string& input_file_extension);

#endif
