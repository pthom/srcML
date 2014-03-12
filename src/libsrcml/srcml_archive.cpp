/**
 * @file srcml_archive.cpp
 *
 * @copyright Copyright (C) 2013-2014 SDML (www.srcML.org)
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
 */

#include <srcml.h>
#include <srcml_types.hpp>
#include <srcMLSAX2Reader.hpp>

#include <srcmlns.hpp>

/**
 * srcml_archive_check_extension
 * @param archive a srcml_archive
 * @param filename name of a file
 *
 * Retrieve the currently registered language for a file extension.
 * Full filename can be provided, and extension will be extracted.
 *
 * @returns the language for extension or if 0 if no language.
 */
const char * srcml_archive_check_extension(srcml_archive * archive, const char* filename) {

    if(archive == NULL || filename == NULL) return 0;

    Language language(Language::getLanguageFromFilename(filename, archive->registered_languages));
    const char * lang_string = language.getLanguageString();
    return strcmp(lang_string, "") == 0 ? 0 : lang_string;

}

/******************************************************************************
 *                                                                            *
 *                        Allocation/cleanup functions                        *
 *                                                                            *
 ******************************************************************************/

/**
 * srcml_create_archive
 *
 * Create a new srcml archive.
 * Client will have to free it using srcml_free_archive().
 *
 * @returns the created archive.
 */
srcml_archive* srcml_create_archive()

{
    srcml_archive * archive;
    try {

        archive = new srcml_archive;

    } catch(...) { return 0; }

    archive->type = SRCML_ARCHIVE_INVALID;
    archive->options = SRCML_OPTION_ARCHIVE | SRCML_OPTION_XML_DECL | SRCML_OPTION_NAMESPACE_DECL | SRCML_OPTION_TIMESTAMP
	| SRCML_OPTION_TIMESTAMP | SRCML_OPTION_HASH;
    archive->tabstop = 8;
    archive->translator = 0;
    archive->reader = 0;
    archive->input = 0;
    archive->close_input = 0;

    // default prefixes
    srcml_archive_register_namespace(archive, SRCML_SRC_NS_PREFIX_DEFAULT, SRCML_SRC_NS_URI);
    srcml_archive_register_namespace(archive, SRCML_CPP_NS_PREFIX_DEFAULT, SRCML_CPP_NS_URI);
    srcml_archive_register_namespace(archive, SRCML_ERR_NS_PREFIX_DEFAULT, SRCML_ERR_NS_URI);
    srcml_archive_register_namespace(archive, SRCML_EXT_LITERAL_NS_PREFIX_DEFAULT, SRCML_EXT_LITERAL_NS_URI);
    srcml_archive_register_namespace(archive, SRCML_EXT_OPERATOR_NS_PREFIX_DEFAULT, SRCML_EXT_OPERATOR_NS_URI);
    srcml_archive_register_namespace(archive, SRCML_EXT_MODIFIER_NS_PREFIX_DEFAULT, SRCML_EXT_MODIFIER_NS_URI);
    srcml_archive_register_namespace(archive, SRCML_EXT_POSITION_NS_PREFIX_DEFAULT, SRCML_EXT_POSITION_NS_URI);

    Language::register_standard_file_extensions(archive->registered_languages);

    return archive;

}

/**
 * srcml_free_archive
 * @param archive a srcml_archive
 *
 * Free a srcml archive that was previously
 * allocated by srcml_create_archive().
 * archive must be reallocated/re-created to use again.
 */
void srcml_free_archive(srcml_archive * archive) {

    if(archive == NULL) return;

    delete archive;
}

/**
 * srcml_clone_archive
 * @param archive a srcml_archive
 *
 * Clone the setup of an existing archive.
 * Client will have to free it using srcml_free_archive().
 *
 * @return the cloned archive
 */
srcml_archive* srcml_clone_archive(const struct srcml_archive* archive) {

    if(archive == NULL) return 0;

    srcml_archive * new_archive = srcml_create_archive();

    if(!new_archive) return 0;

    new_archive->filename = archive->filename;
    new_archive->encoding = archive->encoding;
    new_archive->language = archive->language;
    new_archive->directory = archive->directory;
    new_archive->version = archive->version;

    try {

        for(std::vector<std::string>::size_type pos = 0; pos < archive->attributes.size(); ++pos)
            new_archive->attributes.push_back(archive->attributes.at(pos));

    } catch(...) {}

    new_archive->options = archive->options;
    new_archive->tabstop = archive->tabstop;

    // clear out those added by srcml_create_archive
    for(std::vector<std::string>::size_type pos = 0; pos < archive->namespaces.size(); ++pos) {

        try {

            srcml_archive_register_namespace(new_archive, archive->prefixes.at(pos).c_str(), archive->namespaces.at(pos).c_str());

        } catch(...) {}

    }

    try {
        new_archive->registered_languages.clear();
        for(std::vector<pair>::size_type i = 0; i < archive->registered_languages.size(); ++i)
            new_archive->registered_languages.push_back(archive->registered_languages.at(i));

    } catch(...) {}

    try {

        new_archive->user_macro_list.clear();
        for(std::vector<std::string>::size_type i = 0; i < archive->user_macro_list.size(); ++i)
            new_archive->user_macro_list.push_back(archive->user_macro_list.at(i));

    } catch(...) {}

    return new_archive;

}

/******************************************************************************
 *                                                                            *
 *                           Set up functions                                 *
 *                                                                            *
 ******************************************************************************/

/**
 * srcml_archive_set_src_encoding
 * @param archive a srcml_archive
 * @param encoding an encoding
 *
 * Set the default source encoding of the srcML Archive.
 *
 * @returns SRCML_STATUS_OK on success and SRCML_STATUS_INVALID_ARGUMENT on failure.
 */
int srcml_archive_set_src_encoding(srcml_archive* archive, const char* src_encoding) {

    if(archive == NULL) return SRCML_STATUS_INVALID_ARGUMENT;

    archive->src_encoding = src_encoding ? std::string(src_encoding) : boost::optional<std::string>();

    return SRCML_STATUS_OK;

}

/**
 * srcml_archive_set_encoding
 * @param archive a srcml_archive
 * @param encoding an encoding
 *
 * Set the xml encoding of the srcML Archive.
 *
 * @returns SRCML_STATUS_OK on success and SRCML_STATUS_INVALID_ARGUMENT on failure.
 */
int srcml_archive_set_encoding(srcml_archive* archive, const char* encoding) {

    if(archive == NULL) return SRCML_STATUS_INVALID_ARGUMENT;

    archive->encoding = encoding ? std::string(encoding) : boost::optional<std::string>();

    return SRCML_STATUS_OK;

}

/**
 * srcml_archive_set_language
 * @param archive a srcml_archive
 * @param language a language
 *
 * Set the language of the srcML Archive.
 *
 * @returns SRCML_STATUS_OK on success and SRCML_STATUS_INVALID_ARGUMENT on failure.
 */
int srcml_archive_set_language(srcml_archive* archive, const char* language) {

    if(archive == NULL) return SRCML_STATUS_INVALID_ARGUMENT;

    archive->language = language ? std::string(language) : boost::optional<std::string>();

    return SRCML_STATUS_OK;

}

/**
 * srcml_archive_set_filename
 * @param archive a srcml_archive
 * @param filename the name of a file
 *
 * Set the root filename attribute of the srcML Archive.
 *
 * @returns SRCML_STATUS_OK on success and SRCML_STATUS_INVALID_ARGUMENT on failure.
 */
int srcml_archive_set_filename(srcml_archive* archive, const char* filename) {

    if(archive == NULL) return SRCML_STATUS_INVALID_ARGUMENT;

    archive->filename = filename ? std::string(filename) : boost::optional<std::string>();

    return SRCML_STATUS_OK;

}

/**
 * srcml_archive_set_directory
 * @param archive a srcml_archive
 * @param directory a directory path
 *
 * Set the root directory attribute of the srcML Archive.
 *
 * @returns SRCML_STATUS_OK on success and SRCML_STATUS_INVALID_ARGUMENT on failure.
 */
int srcml_archive_set_directory (srcml_archive* archive, const char* directory) {

    if(archive == NULL) return SRCML_STATUS_INVALID_ARGUMENT;

    archive->directory = directory ? std::string(directory) : boost::optional<std::string>();

    return SRCML_STATUS_OK;

}

/**
 * srcml_archive_set_version
 * @param archive a srcml_archive
 * @param version a version string
 *
 * Set the root version attribute of the srcML Archive.
 *
 * @returns SRCML_STATUS_OK on success and SRCML_STATUS_INVALID_ARGUMENT on failure.
 */
int srcml_archive_set_version(srcml_archive* archive, const char* version) {

    if(archive == NULL) return SRCML_STATUS_INVALID_ARGUMENT;

    archive->version = version ? std::string(version) : boost::optional<std::string>();

    return SRCML_STATUS_OK;

}

/**
 * srcml_archive_set_options
 * @param archive a srcml_archive
 * @param options a set of srcml options
 *
 * Set all options for processing an archive (erases all previous).
 *
 * @returns SRCML_STATUS_OK on success and SRCML_STATUS_INVALID_ARGUMENT on failure.
 */
int srcml_archive_set_options(srcml_archive* archive, unsigned long long options) {

    if(archive == NULL) return SRCML_STATUS_INVALID_ARGUMENT;

    archive->options = options;
    return SRCML_STATUS_OK;

}

/**
 * srcml_archive_enable_option
 * @param archive a srcml_archive
 * @param option a srcml option
 *
 * Enable/set a srcml option, adding to the previously active options.
 * May set multiple options with same call by |ing each.
 *
 * @returns SRCML_STATUS_OK on success and SRCML_STATUS_INVALID_ARGUMENT on failure.
 */
int srcml_archive_enable_option(srcml_archive* archive, unsigned long long option) {

    if(archive == NULL) return SRCML_STATUS_INVALID_ARGUMENT;

    archive->options |= option;
    return SRCML_STATUS_OK;

}

/**
 * srcml_archive_disable_option
 * @param archive a srcml_archive
 * @param option a srcml option
 *
 * Remove a srcml option.
 * May clear multiple options with same call by |ing each.
 *
 * @returns SRCML_STATUS_OK on success and SRCML_STATUS_INVALID_ARGUMENT on failure.
 */
int srcml_archive_disable_option(srcml_archive* archive, unsigned long long option) {

    if(archive == NULL) return SRCML_STATUS_INVALID_ARGUMENT;

    archive->options &= ~option;
    return SRCML_STATUS_OK;

}

/**
 * srcml_archive_set_tabstop
 * @param archive a srcml_archive
 * @param tabstop size of a tabstop
 *
 * Set the tabstop size for position/column calculation.
 *
 * @returns SRCML_STATUS_OK on success and SRCML_STATUS_INVALID_ARGUMENT on failure.
 */
int srcml_archive_set_tabstop(srcml_archive* archive, int tabstop) {

    if(archive == NULL) return SRCML_STATUS_INVALID_ARGUMENT;

    archive->tabstop = tabstop;
    return SRCML_STATUS_OK;

}

/**
 * srcml_archive_register_file_extension
 * @param archive a srcml_archive
 * @param extension a file extension
 * @param language a language
 *
 * Set the given extension to be associate with the given language.
 *
 * @returns SRCML_STATUS_OK on success and a status error code on failure.
 */
int srcml_archive_register_file_extension(srcml_archive* archive, const char* extension, const char* language) {

    if(archive == NULL || extension == NULL || language == NULL)
        return SRCML_STATUS_INVALID_ARGUMENT;

    if(Language::registerUserExt(extension, language, archive->registered_languages))
        return SRCML_STATUS_OK;
    return SRCML_STATUS_INVALID_INPUT;
}

/**
 * srcml_archive_register_namespace
 * @param archive a srcml_archive
 * @param prefix a XML namespace prefix
 * @param ns a XML namespace
 *
 * Create a new namespace or change the prefix of an existing namespace.
 *
 * @returns SRCML_STATUS_OK on success and a status error code on failure.
 */
int srcml_archive_register_namespace(srcml_archive* archive, const char* prefix, const char* ns) {

    if(archive == NULL || prefix == NULL || ns == NULL) return SRCML_STATUS_INVALID_ARGUMENT;

    try {

        for(std::vector<std::string>::size_type i = 0; i < archive->prefixes.size(); ++i)
            if(archive->namespaces.at(i) == ns) {

                archive->prefixes.at(i) = prefix;
                return SRCML_STATUS_OK;
            }

    } catch(...) { return SRCML_STATUS_ERROR; }

    archive->prefixes.push_back(prefix);
    archive->namespaces.push_back(ns);

    return SRCML_STATUS_OK;

}


/**
 * srcml_archive_register_macro
 * @param archive a srcml_archive
 * @param token name of macro
 * @param type macro type
 *
 * Register a macro (token) to be processed as a special type
 *
 * @returns SRCML_STATUS_OK on success and a status error code on failure.
 */
int srcml_archive_register_macro(srcml_archive* archive, const char* token, const char* type) {

    if(archive == NULL || token == NULL || type == NULL) return SRCML_STATUS_INVALID_ARGUMENT;

    try {

        std::vector<std::string>::size_type user_macro_list_size = archive->user_macro_list.size() / 2;
        for(std::vector<std::string>::size_type i = 0; i < user_macro_list_size; ++i)
            if(archive->user_macro_list.at(i * 2) == token) {

                archive->user_macro_list.at(i * 2 + 1) = type;
                return SRCML_STATUS_OK;
            }

    } catch(...) { return SRCML_STATUS_ERROR; }

    archive->user_macro_list.push_back(token);
    archive->user_macro_list.push_back(type);

    return SRCML_STATUS_OK;

}

/******************************************************************************
 *                                                                            *
 *                           Accessor Functions                               *
 *                                                                            *
 ******************************************************************************/

/**
 * srcml_archive_get_src_encoding
 * @param archive a srcml_archive
 *
 * @returns Retrieve the currently default source encoding or NULL.
 */
const char* srcml_archive_get_src_encoding(const struct srcml_archive* archive) {

    return archive && archive->src_encoding ? archive->src_encoding->c_str() : 0;

}

/**
 * srcml_archive_get_encoding
 * @param archive a srcml_archive
 *
 * @returns Retrieve the currently set xml encoding or NULL.
 */
const char* srcml_archive_get_encoding(const struct srcml_archive* archive) {

    return archive && archive->encoding ? archive->encoding->c_str() : 0;

}

/**
 * srcml_archive_get_language
 * @param archive a srcml_archive
 *
 * @returns Retrieve the currently set language or NULL.
 */
const char* srcml_archive_get_language(const struct srcml_archive* archive) {

    return archive && archive->language ? archive->language->c_str() : 0;

}

/**
 * srcml_archive_get_filename
 * @param archive a srcml_archive
 *
 * @returns Retrieve the currently set root filename attribute or NULL.
 */
const char* srcml_archive_get_filename(const struct srcml_archive* archive) {

    return archive && archive->filename ? archive->filename->c_str() : 0;

}

/**
 * srcml_archive_get_directory
 * @param archive a srcml_archive
 *
 * @returns Retrieve the currently set root directory attribute or NULL.
 */
const char* srcml_archive_get_directory(const struct srcml_archive* archive) {

    return archive && archive->directory ? archive->directory->c_str() : 0;

}

/**
 * srcml_archive_get_version
 * @param archive a srcml_archive
 *
 * @returns Retrieve the currently set root version attribute or NULL.
 */
const char* srcml_archive_get_version(const struct srcml_archive* archive) {

    return archive && archive->version ? archive->version->c_str() : 0;

}

/**
 * srcml_archive_get_options
 * @param archive a srcml_archive
 *
 * @returns Retrieve the currently set options.
 */
unsigned long long srcml_archive_get_options(const struct srcml_archive* archive) {

    return archive ? archive->options : 0;

}

/**
 * srcml_archive_get_tabstop
 * @param archive a srcml_archive
 *
 * @returns Retrieve the currently set tabstop size.
 */
int srcml_archive_get_tabstop(const struct srcml_archive* archive) {

    return archive ? archive->tabstop : 0;

}

/**
 * srcml_archive_get_namespace_size
 *
 * @returns Get the number of currently defined namespaces or -1 if archive is NULL
 */
int srcml_archive_get_namespace_size(const struct srcml_archive* archive) {

    return archive ? (int)archive->namespaces.size() : -1;

}

/**
 * srcml_archive_get_namespace_prefix
 * @param pos namespace position
 *
 * @returns Get prefix for the given position on success
 * and NULL on failure.
 */
const char* srcml_archive_get_namespace_prefix(const struct srcml_archive* archive, int pos) {

    if(archive == NULL) return 0;

    try {

        return archive->prefixes.at(pos).c_str();

    } catch(...) {

        return 0;

    }

}

/**
 * srcml_archive_get_prefix_from_uri
 * @param namespace_uri an XML namespace
 *
 * @returns Get the registered prefix for the given namespace
 * on success and NULL on failure.
 */
const char* srcml_archive_get_prefix_from_uri(const struct srcml_archive* archive, const char* namespace_uri) {

    if(archive == NULL || namespace_uri == NULL) return 0;

    try {

        for(std::vector<std::string>::size_type i = 0; i < archive->prefixes.size(); ++i)
            if(archive->namespaces.at(i) == namespace_uri)
                return archive->prefixes.at(i).c_str();

    } catch(...) {}

    return 0;
}

/**
 * srcml_archive_get_namespace_uri
 * @param pos position in namespaces
 *
 * @returns Get the namespace at the given pos on succcess
 * and NULL on failure.
 */
const char* srcml_archive_get_namespace_uri(const struct srcml_archive* archive, int pos) {

    if(archive == NULL) return 0;

    try {

        return archive->namespaces.at(pos).c_str();

    } catch (...) {

        return 0;

    }

}

/**
 * srcml_archive_get_uri_from_prefix
 * @param prefix an XML prefix
 *
 * @returns Get the first namespace for the given prefix on success
 * and NULL on failure.
 */
const char* srcml_archive_get_uri_from_prefix(const struct srcml_archive* archive, const char* prefix) {

    if(archive == NULL || prefix == NULL) return 0;

    try {

        for(std::vector<std::string>::size_type i = 0; i < archive->namespaces.size(); ++i)
            if(archive->prefixes.at(i) == prefix)
                return archive->namespaces.at(i).c_str();

    } catch(...) {}

    return 0;

}

/**
 * srcml_archive_get_macro_list_size
 *
 * @returns Get the number of currently defined macros or -1 if archive is NULL
 */
int srcml_archive_get_macro_list_size(const struct srcml_archive* archive) {

    return archive ? (int)(archive->user_macro_list.size() / 2) : -1;

}

/**
 * srcml_archive_get_macro_token
 * @param pos macro position
 *
 * @returns Get token for the given position on success
 * and NULL on failure.
 */
const char* srcml_archive_get_macro_token(const struct srcml_archive* archive, int pos) {

    if(archive == NULL) return 0;

    try {

        return archive->user_macro_list.at(pos * 2).c_str();

    } catch(...) {

        return 0;

    }

}

/**
 * srcml_archive_get_macro_token_type
 * @param token a macro token
 *
 * @returns Get the registered type for the given token
 * on success and NULL on failure.
 */
const char* srcml_archive_get_macro_token_type(const struct srcml_archive* archive, const char* token) {

    if(archive == NULL || token == NULL) return 0;

    try {

        std::vector<std::string>::size_type user_macro_list_size = archive->user_macro_list.size() / 2;
        for(std::vector<std::string>::size_type i = 0;  user_macro_list_size; ++i)
            if(archive->user_macro_list.at(i * 2) == token)
                return archive->user_macro_list.at(i * 2 + 1).c_str();

    } catch(...) {}

    return 0;
}

/**
 * srcml_archive_get_macro_type
 * @param pos position in macro list
 *
 * @returns Get the type at the given pos on succcess
 * and NULL on failure.
 */
const char* srcml_archive_get_macro_type(const struct srcml_archive* archive, int pos) {

    if(archive == NULL) return 0;

    try {

        return archive->user_macro_list.at(pos * 2 + 1).c_str();

    } catch (...) {

        return 0;

    }

}

/******************************************************************************
 *                                                                            *
 *                       Archive write open functions                         *
 *                                                                            *
 ******************************************************************************/

/**
 * srcml_archive_open_filename
 * @param archive a srcml_archive
 * @param srcml_filename name of an output file
 *
 * Open up a srcml_archive for writing.  Set the output
 * to go to the file srcml_filename.
 *
 * @returns Return SRCML_STATUS_OK on success and a status error code on failure.
 */
int srcml_write_open_filename(srcml_archive* archive, const char* srcml_filename) {

    if(archive == NULL || srcml_filename == NULL) return SRCML_STATUS_INVALID_ARGUMENT;

    archive->type = SRCML_ARCHIVE_WRITE;

    if(archive->user_macro_list.size()) archive->options |= OPTION_MACRO_LIST;

    try {

        archive->translator = new srcMLTranslator(srcml_check_language(archive->language ? archive->language->c_str() : 0),
                                                  archive->src_encoding ? archive->src_encoding->c_str() : "ISO-8859-1",
                                                  archive->encoding ? archive->encoding->c_str() : "UTF-8",
                                                  srcml_filename,
                                                  archive->options,
                                                  archive->directory ? archive->directory->c_str() : 0,
                                                  archive->filename ? archive->filename->c_str() : 0,
                                                  archive->version ? archive->version->c_str() : 0,
                                                  &archive->prefixes.front(),
                                                  archive->tabstop);
        archive->translator->setMacroList(archive->user_macro_list);

    } catch(...) { return SRCML_STATUS_IO_ERROR; }

    return SRCML_STATUS_OK;

}

/**
 * srcml_archive_open_memory
 * @param archive a srcml_archive
 * @param buffer location to return output string
 *
 * Open up a srcml_archive for writing.  Set the output
 * to be to memory.  Buffer is allocated and set to the location
 * buffer points at.  Buffer must be freed after use.
 *
 * @returns Return SRCML_STATUS_OK on success and a status error code on failure.
 */
int srcml_write_open_memory(srcml_archive* archive, char** buffer, int * size) {

    if(archive == NULL || buffer == NULL || size == NULL) return SRCML_STATUS_INVALID_ARGUMENT;

    archive->type = SRCML_ARCHIVE_WRITE;

    if(archive->user_macro_list.size()) archive->options |= OPTION_MACRO_LIST;

    try {

        archive->translator = new srcMLTranslator(srcml_check_language(archive->language ? archive->language->c_str() : 0),
                                                  archive->src_encoding ? archive->src_encoding->c_str() : "ISO-8859-1",
                                                  archive->encoding ? archive->encoding->c_str() : "UTF-8",
                                                  buffer,
                                                  size,
                                                  archive->options,
                                                  archive->directory ? archive->directory->c_str() : 0,
                                                  archive->filename ? archive->filename->c_str() : 0,
                                                  archive->version ? archive->version->c_str() : 0,
                                                  &archive->prefixes.front(),
                                                  archive->tabstop);

        archive->translator->setMacroList(archive->user_macro_list);

    } catch(...) { return SRCML_STATUS_IO_ERROR; }

    return SRCML_STATUS_OK;

}

/**
 * srcml_archive_open_FILE
 * @param archive a srcml_archive
 * @param srcml_file FILE opened for writing
 *
 * Open up a srcml_archive for writing.  Set the output
 * to go to the FILE srcml_file.
 *
 * @returns Return SRCML_STATUS_OK on success and a status error code on failure.
 */
int srcml_write_open_FILE(srcml_archive* archive, FILE* srcml_file) {

    if(archive == NULL || srcml_file == NULL) return SRCML_STATUS_INVALID_ARGUMENT;

    xmlOutputBufferPtr output_buffer = xmlOutputBufferCreateFile(srcml_file, xmlFindCharEncodingHandler(archive->encoding ? archive->encoding->c_str() : 0));
    if(output_buffer == NULL) return SRCML_STATUS_IO_ERROR;

    archive->type = SRCML_ARCHIVE_WRITE;

    if(archive->user_macro_list.size()) archive->options |= OPTION_MACRO_LIST;

    try {

        archive->translator = new srcMLTranslator(srcml_check_language(archive->language ? archive->language->c_str() : 0),
                                                  archive->src_encoding ? archive->src_encoding->c_str() : "ISO-8859-1",
                                                  archive->encoding ? archive->encoding->c_str() : "UTF-8",
                                                  output_buffer,
                                                  archive->options,
                                                  archive->directory ? archive->directory->c_str() : 0,
                                                  archive->filename ? archive->filename->c_str() : 0,
                                                  archive->version ? archive->version->c_str() : 0,
                                                  &archive->prefixes.front(),
                                                  archive->tabstop);

        archive->translator->setMacroList(archive->user_macro_list);

    } catch(...) {

        xmlOutputBufferClose(output_buffer);
        return SRCML_STATUS_IO_ERROR;

    }

    return SRCML_STATUS_OK;

}

/**
 * srcml_archive_open_fd
 * @param archive a srcml_archive
 * @param srcml_fd output file descriptor
 *
 * Open up a srcml_archive for writing.  Set the output
 * to go to the file descriptor srcml_fd.
 *
 * @returns Return SRCML_STATUS_OK on success and a status error code on failure.
 */
int srcml_write_open_fd(srcml_archive* archive, int srcml_fd) {

    if(archive == NULL || srcml_fd < 0) return SRCML_STATUS_INVALID_ARGUMENT;

    xmlOutputBufferPtr output_buffer = xmlOutputBufferCreateFd(srcml_fd, xmlFindCharEncodingHandler(archive->encoding ? archive->encoding->c_str() : 0));
    if(output_buffer == NULL) return SRCML_STATUS_IO_ERROR;

    archive->type = SRCML_ARCHIVE_WRITE;

    if(archive->user_macro_list.size()) archive->options |= OPTION_MACRO_LIST;

    try {

        archive->translator = new srcMLTranslator(srcml_check_language(archive->language ? archive->language->c_str() : 0),
                                                  archive->src_encoding ? archive->src_encoding->c_str() : "ISO-8859-1",
                                                  archive->encoding ? archive->encoding->c_str() : "UTF-8",
                                                  output_buffer,
                                                  archive->options,
                                                  archive->directory ? archive->directory->c_str() : 0,
                                                  archive->filename ? archive->filename->c_str() : 0,
                                                  archive->version ? archive->version->c_str() : 0,
                                                  &archive->prefixes.front(),
                                                  archive->tabstop);

        archive->translator->setMacroList(archive->user_macro_list);

    } catch(...) {

        xmlOutputBufferClose(output_buffer);
        return SRCML_STATUS_IO_ERROR;

    }

    return SRCML_STATUS_OK;

}

/******************************************************************************
 *                                                                            *
 *                       Archive read open functions                          *
 *                                                                            *
 ******************************************************************************/

/**
 * srcml_read_internal
 * @param archive a srcml_archive
 *
 * Function used internally to the srcml_read_open_* functions.
 * Reads and sets the open type as well as gathers the attributes
 * and sets the options from the opened srcML Archive.
 */
static void srcml_read_internal(srcml_archive * archive) {

    archive->type = SRCML_ARCHIVE_READ;

    boost::optional<std::string> language, filename, directory, version;
    bool done = !archive->reader->readRootUnitAttributes(language, filename, directory, version,
                                                         archive->attributes, archive->prefixes,
                                                         archive->namespaces,
                                                         archive->options,
                                                         archive->tabstop,
                                                         archive->user_macro_list);
    if(!done) {

        archive->language = language;
        archive->filename = filename;
        archive->directory = directory;
        archive->version = version;

    }

}

/**
 * srcml_read_open_filename
 * @param archive a srcml_archive
 * @param srcml_filename name of an input file
 *
 * Open a srcML archive for reading.  Set the input to be read from
 * srcml_filename.
 *
 * @returns Return SRCML_STATUS_OK on success and a status error code on failure.
 */
int srcml_read_open_filename(srcml_archive* archive, const char* srcml_filename) {

    if(archive == NULL || srcml_filename == NULL) return SRCML_STATUS_INVALID_ARGUMENT;

    archive->input = xmlParserInputBufferCreateFilename(srcml_filename, XML_CHAR_ENCODING_NONE);
    try {

        archive->reader = new srcMLSAX2Reader(archive->input);

    } catch(...) {

        xmlFreeParserInputBuffer(archive->input);
        return SRCML_STATUS_IO_ERROR;

    }

    srcml_read_internal(archive);
    archive->close_input = true;

    return SRCML_STATUS_OK;

}

/**
 * srcml_read_open_memory
 * @param archive a srcml_archive
 * @param buffer an input buffer
 * @param buffer_size size of the input buffer
 *
 * Open a srcML archive for reading.  Set the input to be read from
 * the buffer up until buffer_size.
 *
 * @returns Return SRCML_STATUS_OK on success and a status error code on failure.
 */
int srcml_read_open_memory(srcml_archive* archive, const char* buffer, size_t buffer_size) {

    if(archive == NULL || buffer == NULL || buffer_size <= 0) return SRCML_STATUS_INVALID_ARGUMENT;

    archive->input = xmlParserInputBufferCreateMem(buffer, (int)buffer_size, XML_CHAR_ENCODING_NONE);
    try {

        archive->reader = new srcMLSAX2Reader(archive->input);

    } catch(...) {

        xmlFreeParserInputBuffer(archive->input);
        return SRCML_STATUS_IO_ERROR;

    }

    srcml_read_internal(archive);
    archive->close_input = true;

    return SRCML_STATUS_OK;

}

/**
 * srcml_read_open_FILE
 * @param archive a srcml_archive
 * @param srcml_file a FILE opened for reading
 *
 * Open a srcML archive for reading.  Set the input to be read from
 * the FILE srcml_file.
 *
 * @returns Return SRCML_STATUS_OK on success and a status error code on failure.
 */
int srcml_read_open_FILE(srcml_archive* archive, FILE* srcml_file) {

    if(archive == NULL || srcml_file == NULL) return SRCML_STATUS_INVALID_ARGUMENT;

    archive->input = xmlParserInputBufferCreateFile(srcml_file, XML_CHAR_ENCODING_NONE);
    try {

        archive->reader = new srcMLSAX2Reader(archive->input);

    } catch(...) {

        xmlFreeParserInputBuffer(archive->input);
        return SRCML_STATUS_IO_ERROR;

    }

    srcml_read_internal(archive);
    archive->close_input = false;

    return SRCML_STATUS_OK;

}

/**
 * srcml_read_open_fd
 * @param archive a srcml_archive
 * @param srcml_fd a file descriptor opened for reading
 *
 * Open a srcML archive for reading.  Set the input to be read from
 * the file descriptor srcml_fd.
 *
 * @returns Return SRCML_STATUS_OK on success and a status error code on failure.
 */
int srcml_read_open_fd(srcml_archive* archive, int srcml_fd) {

    if(archive == NULL || srcml_fd < 0) return SRCML_STATUS_INVALID_ARGUMENT;

    archive->input = xmlParserInputBufferCreateFd(srcml_fd, XML_CHAR_ENCODING_NONE);
    try {

        archive->reader = new srcMLSAX2Reader(archive->input);

    } catch(...) {

        xmlFreeParserInputBuffer(archive->input);
        return SRCML_STATUS_IO_ERROR;

    }

    srcml_read_internal(archive);
    archive->close_input = false;

    return SRCML_STATUS_OK;

}

/******************************************************************************
 *                                                                            *
 *                       Archive read/write unit functions                    *
 *                                                                            *
 ******************************************************************************/

/**
 * srcml_write_unit
 * @param archive a srcml archive opened for writing
 * @param unit a srcml_unit to output
 *
 * Append the srcml_unit unit to the srcml_archive archive.
 * If copying from a read and only the attributes have been read
 * read in the xml and output.
 *
 * @returns Return SRCML_STATUS_OK on success and a status error code on failure.
 */
int srcml_write_unit(srcml_archive* archive, const struct srcml_unit* unit) {

    if(archive == NULL || unit == NULL) return SRCML_STATUS_INVALID_ARGUMENT;

    if(!unit->unit && !unit->read_header) return SRCML_STATUS_UNINITIALIZED_UNIT;

    boost::optional<std::string> read_unit;
    if(!unit->unit && (unit->archive->type == SRCML_ARCHIVE_READ || unit->archive->type == SRCML_ARCHIVE_RW))
        unit->archive->reader->readsrcML(read_unit);

    if(!unit->unit && !read_unit) return SRCML_STATUS_UNINITIALIZED_UNIT;

    if(archive->type != SRCML_ARCHIVE_WRITE && archive->type != SRCML_ARCHIVE_RW) return SRCML_STATUS_INVALID_IO_OPERATION;
    archive->translator->add_unit(read_unit ? read_unit->c_str() : unit->unit->c_str(),
				  unit->output_hash && unit->hash ? unit->hash->c_str() : 0);

    return SRCML_STATUS_OK;
}

/**
 * srcml_read_unit_header
 * @param archive a srcml archive open for reading
 *
 * Read the next unit from the archive.
 * unit contains read attribute.  xml is set to read only if needed
 * in a subsequent call.
 *
 * @returns Return the read srcml_unit on success.
 * On failure returns NULL.
 */
srcml_unit* srcml_read_unit_header(srcml_archive* archive) {

    if(archive == NULL) return 0;

    if(archive->type != SRCML_ARCHIVE_READ && archive->type != SRCML_ARCHIVE_RW) return 0;

    srcml_unit * unit = srcml_create_unit(archive);
    int not_done = archive->reader->readUnitAttributes(unit->language, unit->filename, unit->directory, unit->version);

    if(!not_done) {
        srcml_free_unit(unit);
        return 0;
    }

    unit->read_header = true;

    return unit;
}

/**
 * srcml_read_unit_xml
 * @param archive a srcml archive open for reading
 *
 * Read the next unit from the archive.
 * unit contains the complete srcml only.
 *
 * @returns Return the read srcml_unit on success.
 * On failure returns NULL.
 */
srcml_unit* srcml_read_unit_xml(srcml_archive* archive) {

    if(archive == NULL) return 0;

    if(archive->type != SRCML_ARCHIVE_READ && archive->type != SRCML_ARCHIVE_RW) return 0;

    srcml_unit * unit = srcml_create_unit(archive);
    int not_done = 0;
    if(!unit->read_header)
        not_done = archive->reader->readUnitAttributes(unit->language, unit->filename, unit->directory, unit->version);
    archive->reader->readsrcML(unit->unit);

    if(!not_done || !unit->unit) {
        srcml_free_unit(unit);
        unit = 0;
    }

    return unit;
}


/**
 * srcml_read_unit
 * @param archive a srcml archive open for reading
 *
 * Read the next unit from the archive.
 * unit contains read attributes and complete srcml.
 *
 * @returns Return the read srcml_unit on success.
 * On failure returns NULL.
 */
srcml_unit* srcml_read_unit(srcml_archive* archive) {

    if(archive == NULL) return 0;

    if(archive->type != SRCML_ARCHIVE_READ && archive->type != SRCML_ARCHIVE_RW) return 0;

    srcml_unit * unit = srcml_create_unit(archive);
    int not_done = 0;
    if(!unit->read_header)
        not_done = archive->reader->readUnitAttributes(unit->language, unit->filename, unit->directory, unit->version);
    archive->reader->readsrcML(unit->unit);

    if(!not_done || !unit->unit) {
        srcml_free_unit(unit);
        unit = 0;
    }

    return unit;
}

/**
 * srcml_skip_unit
 * @param archive a srcml archive to read from
 *
 *  Read over (skipping) the next unit from the archive
 *
 *  @returns Returns 0 if no unit to skip, 1 otherwise
 */
int srcml_skip_unit(srcml_archive* archive) {

    if(archive == NULL) return 0;

    if(archive->type != SRCML_ARCHIVE_READ && archive->type != SRCML_ARCHIVE_RW) return 0;

    srcml_unit * unit = srcml_read_unit_header(archive);
    if(unit == 0) return 0;

    srcml_free_unit(unit);

    return 1;
}

/**
 * srcml_read_unit_position
 * @param archive a srcml_archive
 * @param pos a relative position in archive
 *
 * Read a unit at a specific relative position in an archive
 * Unit numbers are positive and start at 1.  srcml_read_unit_position(archive, 1)
 * is equivalent to srcml_read_unit.
 *
 * @returns Returns 0 if pos unit does not exist and the read unit otherwise.
 */
srcml_unit* srcml_read_unit_position(srcml_archive* archive, int pos) {

    if(archive == NULL || pos <= 0) return 0;

    if(archive->type != SRCML_ARCHIVE_READ && archive->type != SRCML_ARCHIVE_RW) return 0;

    for(; pos != 1 && srcml_skip_unit(archive); --pos)
        ;

    if(pos != 1) return 0;

    return srcml_read_unit(archive);

}

/******************************************************************************
 *                                                                            *
 *                       Archive close function                               *
 *                                                                            *
 ******************************************************************************/

/**
 * srcml_close_archive
 * @param archive an open srcml archive
 *
 * Close a srcML archive opened using srcml_read_open_*
 * or srcml_write_open_*.
 * Archive can be reopened.
 */
void srcml_close_archive(srcml_archive * archive) {

    if(archive == NULL) return;

    if(archive->translator) archive->translator->close();
    if(archive->translator) delete archive->translator, archive->translator = 0;
    if(archive->reader) delete archive->reader, archive->reader = 0;
    if(archive->input) {
        if(!archive->close_input) archive->input->closecallback = 0;
        xmlFreeParserInputBuffer(archive->input);
        archive->input = 0;
    }

    archive->type = SRCML_ARCHIVE_INVALID;

}
