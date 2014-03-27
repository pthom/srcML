/**
 * @file srcMLTranslator.cpp
 *
 * @copyright Copyright (C) 2003-2014 SDML (www.srcML.org)
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
 */

/*
  Class for straightforward translation from source code to srcML
*/

#include "srcMLTranslator.hpp"
#include "KeywordLexer.hpp"
#include "srcMLParser.hpp"
#include "StreamMLParser.hpp"
#include "srcMLOutput.hpp"
#include "srcmlns.hpp"
#ifdef _MSC_BUILD
#define strdup _strdup
#endif

/// constructor
srcMLTranslator::srcMLTranslator(int language,                // programming language of source code
                                 const char* src_encoding,    // text encoding of source code
                                 const char* xml_encoding,    // xml encoding of result srcML file
                                 const char* srcml_filename,  // filename of result srcML file
                                 OPTION_TYPE& op,             // many and varied options
                                 const char* directory,       // root unit directory
                                 const char* filename,        // root unit filename
                                 const char* version,         // root unit version
                                 std::string * uri,           // uri prefixes
                                 int tabsize                  // size of tabs
                                 )
    : Language(language), pinput(0), first(true),
      root_directory(directory), root_filename(filename), root_version(version),
      encoding(src_encoding), xml_encoding(xml_encoding), options(op), buffer(0),
      out(0, srcml_filename, getLanguageString(), xml_encoding, options, uri, tabsize, 0), tabsize(tabsize), uri(uri),
      str_buffer(0), size(0) {}

// constructor
srcMLTranslator::srcMLTranslator(int language,                // programming language of source code
                                 const char* src_encoding,    // text encoding of source code
                                 const char* xml_encoding,    // xml encoding of result srcML file
                                 char ** str_buf,
                                 int * size,
                                 OPTION_TYPE & op,             // many and varied options
                                 const char* directory,       // root unit directory
                                 const char* filename,        // root unit filename
                                 const char* version,         // root unit version
                                 std::string * uri,           // uri prefixes
                                 int tabsize                  // size of tabs
                                 )
    :  Language(language), pinput(0), first(true), root_directory(directory), root_filename(filename), root_version(version),
       encoding(src_encoding), xml_encoding(xml_encoding), options(op), buffer(0),
       out(0, 0, getLanguageString(), xml_encoding, options, uri, tabsize, 0), tabsize(tabsize),
       uri(uri), str_buffer(str_buf), size(size) {

    buffer = xmlBufferCreate();
    xmlOutputBufferPtr obuffer = xmlOutputBufferCreateBuffer(buffer, xmlFindCharEncodingHandler(xml_encoding));
    out.setOutputBuffer(obuffer);

}

// constructor
srcMLTranslator::srcMLTranslator(int language,                // programming language of source code
                                 const char* src_encoding,    // text encoding of source code
                                 const char* xml_encoding,    // xml encoding of result srcML file
                                 xmlOutputBuffer * output_buffer,
                                 OPTION_TYPE& op,             // many and varied options
                                 const char* directory,       // root unit directory
                                 const char* filename,        // root unit filename
                                 const char* version,         // root unit version
                                 std::string * uri,           // uri prefixes
                                 int tabsize                  // size of tabs
                                 )
    : Language(language), pinput(0), first(true),
      root_directory(directory), root_filename(filename), root_version(version),
      encoding(src_encoding), xml_encoding(xml_encoding), options(op), buffer(0),
      out(0, 0, getLanguageString(), xml_encoding, options, uri, tabsize, output_buffer), tabsize(tabsize), uri(uri),
      str_buffer(0), size(0) {}

void srcMLTranslator::setMacroList(std::vector<std::string> & list) {
    user_macro_list = list;
    out.setMacroList(list);
}

// translate from input stream to output stream
void srcMLTranslator::setInput(const char* path) {

    try {

        pinput = new UTF8CharBuffer(path, encoding, 0);

    } catch (const std::exception& e) {
        fprintf(stderr, "SRCML Exception: %s\n", e.what());
    }
    catch (UTF8FileError) {
        throw FileError();
    }
    catch (...) {
        fprintf(stderr, "ERROR\n");
    }

}
// close the output
void srcMLTranslator::close() {

    if(first && (options & OPTION_ARCHIVE) > 0) {

        // Open for write;
        out.initWriter();

        out.outputXMLDecl();

        // root unit for compound srcML documents
        out.startUnit(0, root_directory, root_filename, root_version, 0, 0, true);

    }

    out.close();
}

// translate from input stream to output stream
void srcMLTranslator::translate(const char* unit_directory,
                                const char* unit_filename,
                                const char* unit_version,
                                const char* unit_timestamp,
                                const char* unit_hash,
                                int language) {

    if(first) {

        out.initWriter();

        out.outputXMLDecl();

        // root unit for compound srcML documents
        if((options & OPTION_ARCHIVE) > 0)
            out.startUnit(0, root_directory, root_filename, root_version, 0, 0, true);

    }

    first = false;

    try {

        // master lexer with multiple streams
        antlr::TokenStreamSelector selector;

        // srcML lexical analyzer from standard input
        KeywordLexer lexer(pinput, language, options, user_macro_list);
        lexer.setSelector(&selector);
        lexer.setTabsize(tabsize);

        // pure block comment lexer
        CommentTextLexer textlexer(lexer.getInputState());
        textlexer.setSelector(&selector);

        // switching between lexers
        selector.addInputStream(&lexer, "main");
        selector.addInputStream(&textlexer, "text");
        selector.select(&lexer);

        // base stream parser srcML connected to lexical analyzer
        StreamMLParser<srcMLParser> parser(selector, language, options);

        // connect local parser to attribute for output
        out.setTokenStream(parser);

        // parse and form srcML output with unit attributes
        Language l(language);
        out.consume(l.getLanguageString(), unit_directory, unit_filename, unit_version, unit_timestamp, unit_hash);

    } catch (const std::exception& e) {
        fprintf(stderr, "SRCML Exception: %s\n", e.what());
    }
    catch (UTF8FileError) {
        throw FileError();
    }
    catch (...) {
        fprintf(stderr, "ERROR\n");
    }
}

// translate from input stream to output stream separate of current output stream
void srcMLTranslator::translate_separate(const char* unit_directory,
                                         const char* unit_filename,
                                         const char* unit_version,
                                         const char* unit_timestamp,
                                         const char* unit_hash,
                                         int language, UTF8CharBuffer * parser_input, xmlBuffer* output_buffer,
                                         OPTION_TYPE translation_options) {

    xmlOutputBufferPtr obuffer = xmlOutputBufferCreateBuffer(output_buffer, xmlFindCharEncodingHandler("UTF-8"));
    srcMLOutput sep_out(0, 0, getLanguageString(), xml_encoding, translation_options, uri, tabsize, obuffer);
    sep_out.initWriter();
    sep_out.setMacroList(user_macro_list);

    // save old output
    if(isoption(translation_options, OPTION_ARCHIVE))
        sep_out.setDepth(1);

    try {

        // master lexer with multiple streams
        antlr::TokenStreamSelector selector;

        // srcML lexical analyzer from standard input
        KeywordLexer lexer(parser_input, language, translation_options, user_macro_list);
        lexer.setSelector(&selector);
        lexer.setTabsize(tabsize);

        // pure block comment lexer
        CommentTextLexer textlexer(lexer.getInputState());
        textlexer.setSelector(&selector);

        // switching between lexers
        selector.addInputStream(&lexer, "main");
        selector.addInputStream(&textlexer, "text");
        selector.select(&lexer);

        // base stream parser srcML connected to lexical analyzer
        StreamMLParser<srcMLParser> parser(selector, language, translation_options);

        // connect local parser to attribute for output
        sep_out.setTokenStream(parser);

        // parse and form srcML output with unit attributes
        Language l(language);
        sep_out.consume(l.getLanguageString(), unit_directory, unit_filename, unit_version, unit_timestamp, unit_hash);

    } catch (const std::exception& e) {
        fprintf(stderr, "SRCML Exception: %s\n", e.what());
    }
    catch (UTF8FileError) {
        throw FileError();
    }
    catch (...) {
        fprintf(stderr, "ERROR\n");
    }

    sep_out.setDepth(0);

}

void srcMLTranslator::add_unit(std::string xml, const char * hash) {

    if(first) {

        // Open for write;
        out.initWriter();

        out.outputXMLDecl();

        // root unit for compound srcML documents
        if((options & OPTION_ARCHIVE) > 0)
            out.startUnit(0, root_directory, root_filename, root_version, 0, 0, true);

        if ((options & OPTION_ARCHIVE) > 0)
            out.processText("\n\n", 2);

    }

    first = false;

    char * cxml = (char *)xml.c_str();

    if(!isoption(options, OPTION_ARCHIVE)) {

        char * pos = strchr(cxml, '>');
        if(pos == 0) return;

        char * src_ns_pos = strnstr(cxml, SRCML_SRC_NS_URI, pos - cxml);

        if(src_ns_pos == 0) {

            char * unit_pos = strstr(cxml, "unit");

            std::string ns = " xmlns";

            if((unit_pos - cxml) != 1) {

                ns += ":";
                char * colon_pos = strchr(cxml, ':');
                colon_pos[0] = 0;
                ns += (cxml + 1);
                colon_pos[0] = ':';

            }

            ns += "=\"";
            ns += SRCML_SRC_NS_URI;
            ns += "\"";

            // write out up to unit
            xmlTextWriterWriteRawLen(out.getWriter(), (xmlChar *)cxml, (int)((unit_pos + 4) - cxml));

            // write out namespace declaration
            xmlTextWriterWriteRaw(out.getWriter(), (xmlChar *)ns.c_str());

            // update pointer for remaining
            cxml = unit_pos + 4;

        }

    } 

    if(hash) {

        char * pos = strchr(cxml, '>');
        if(pos == 0) return;

        char * hash_pos = strnstr(cxml, "hash", pos - cxml);


        if(hash_pos != 0) {

            xmlTextWriterWriteRawLen(out.getWriter(), (xmlChar *)cxml, (int)((hash_pos + 6) - cxml));
            xmlTextWriterWriteRaw(out.getWriter(), (xmlChar *)hash);
            cxml = hash_pos + 6;

            // consume hash if already there this is generic may consider using just 20 for standard hash size
            if(cxml[0] != '"')
                cxml = strchr(cxml, '"');

        }


    } 

    xmlTextWriterWriteRaw(out.getWriter(), (xmlChar *)cxml);


    if ((options & OPTION_ARCHIVE) > 0)
        out.processText("\n\n", 2);


}

// destructor
srcMLTranslator::~srcMLTranslator() {

    if(str_buffer && buffer->use) {

        (*str_buffer) = strdup((const char *)buffer->content);
        if(size && *str_buffer) *size = (int)buffer->use;

    }

    if(buffer)
        xmlBufferFree(buffer);

}
