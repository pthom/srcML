/*
  src2srcml.cpp

  Copyright (C) 2002-2010  SDML (www.sdml.info)

  This file is part of the srcML translator.

  The srcML translator is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  The srcML translator is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with the srcML translator; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  Main program to run the srcML translator.
*/

#include <cstring>
#include <sys/stat.h>
#include "version.h"
#include "srcmlapps.h"
#include "srcmlns.h"
#include "Options.h"
#include "project.h"
#include "Language.h"
#include "srcMLTranslator.h"
#include "URIStream.h"
#include <getopt.h>

int option_error_status(int optopt);

using namespace LanguageName;

const char* const NAME = "src2srcml";

const char* const DEBUG_FLAG = "--debug";
const char* const DEBUG_FLAG_SHORT = "-g";

const char* const LITERAL_FLAG = "--literal";

const char* const OPERATOR_FLAG = "--operator";

const char* const MODIFIER_FLAG = "--modifier";

const char* const INTERACTIVE_FLAG = "--interactive";
const char* const INTERACTIVE_FLAG_SHORT = "-c";

const char* const CPP_MARKUP_ELSE_FLAG = "--cpp_markup_else";
const char* const CPP_MARKUP_ELSE_FLAG_SHORT = "";

const char* const CPP_TEXTONLY_ELSE_FLAG = "--cpp_text_else";
const char* const CPP_TEXTONLY_ELSE_FLAG_SHORT = "";

const char* const CPP_MARKUP_IF0_FLAG = "--cpp_markup_if0";
const char* const CPP_MARKUP_IF0_FLAG_SHORT = "";

const char* const CPP_TEXTONLY_IF0_FLAG = "--cpp_text_if0";
const char* const CPP_TEXTONLY_IF0_FLAG_SHORT = "";

const char* const EXPRESSION_MODE_FLAG = "--expression";
const char* const EXPRESSION_MODE_FLAG_SHORT = "-e";

const char* const SELF_VERSION_FLAG = "--self-version";
const char* const SELF_VERSION_FLAG_SHORT = "";

const char* const DEFAULT_XML_ENCODING = "UTF-8";

const char* const FILELIST_FLAG = "--files-from";
const char* const FILELIST_FLAG_SHORT = "";

const char* const XMLNS_FLAG = "--xmlns";
const char* const XMLNS_DEFAULT_FLAG_FULL = "--xmlns=URI";
const char* const XMLNS_FLAG_FULL = "--xmlns:PREFIX=URI";

const int DEFAULT_LANGUAGE = srcMLTranslator::LANGUAGE_CXX;

const char* const EXAMPLE_TEXT_FILENAME="foo.cpp";
const char* const EXAMPLE_XML_FILENAME="foo.cpp.xml";

const char FILELIST_COMMENT = '#';

enum {
  SRCML_SRC_NS_URI_POS = 0, 
  SRCML_CPP_NS_URI_POS,
  SRCML_ERR_NS_URI_POS,
  SRCML_EXT_LITERAL_NS_URI_POS,
  SRCML_EXT_OPERATOR_NS_URI_POS,
  SRCML_EXT_MODIFIER_NS_URI_POS,
};

char const * const num2uri[] = {
  SRCML_SRC_NS_URI,
  SRCML_CPP_NS_URI,
  SRCML_ERR_NS_URI,
  SRCML_EXT_LITERAL_NS_URI,
  SRCML_EXT_OPERATOR_NS_URI,
  SRCML_EXT_MODIFIER_NS_URI,
};

// output help
void output_help(const char* name) {
  printf( "Usage: %s [options] <infile>... [-o <outfile>]\n\n"

	  "Translates source-code files in C, C++, and Java into the XML source-code representation srcML.\n"
	  //	  "with support for creating compound srcML documents.\n\n"
	  "Both single and multiple files can be stored in a single srcML documents.\n\n"

	  "All output is to standard output, unless '-o' option is given.\n"
	  "When no filenames are given read from standard input and write to standard output.\n"
	  "An input filename of '-' also reads from standard input.\n\n"

	  "Options:\n", name);

  printf("  %s, %-16s display this help and exit\n",      HELP_FLAG_SHORT, HELP_FLAG);
  printf("  %s, %-16s display version number and exit\n\n", VERSION_FLAG_SHORT, VERSION_FLAG);

  printf("  %s, %-16s write result to <file|URI> instead of standard output\n", OUTPUT_FLAG_SHORT, OUTPUT_FLAG);
  printf("  %-20s read list of source file names from <file|URI> to form a compound srcML document\n\n",
	  FILELIST_FLAG);

  printf("  %s, %-16s store all input source files in one compound srcML document\n",
	  NESTED_FLAG_SHORT, NESTED_FLAG);

  printf("  %s, %-16s translates a single, individual expression outside of any statement\n",
	  EXPRESSION_MODE_FLAG_SHORT, EXPRESSION_MODE_FLAG);

  printf("  %s, %-16s set the output XML encoding to ENC (default:  %s)\n",
	  ENCODING_FLAG_SHORT, ENCODING_FLAG, DEFAULT_XML_ENCODING);

  printf("  %s, %-16s set the input source encoding to ENC (default:  %s)\n\n",
	  TEXTENCODING_FLAG_SHORT, TEXTENCODING_FLAG, DEFAULT_TEXT_ENCODING);

      /*
	      << "  " << SKIP_ENCODING_FLAG_SHORT        << ", " << setw(COL) <<  SKIP_ENCODING_FLAG
	      << "store the text without any text encoding changes" << "\n"
      */

  printf("  %-23s set the default namespace URI\n", XMLNS_DEFAULT_FLAG_FULL);
  printf("        (default:  xmlns=\"%s\")\n", SRCML_SRC_NS_URI);

  printf("  %-23s set the namespace PREFIX for the namespace URI\n", XMLNS_FLAG_FULL);
  printf("        (defaults:  xmlns:cpp=\"%s\")\n", SRCML_CPP_NS_URI);
  printf("        (defaults:  xmlns:srcerr=\"%s\")\n", SRCML_ERR_NS_URI);

  printf("  %-23s	do not output the default XML declaration\n", NO_XML_DECLARATION_FLAG);
  printf("  %-23s	do not output any namespace declarations\n\n", NO_NAMESPACE_DECLARATION_FLAG);

  printf("  %s, %-16s output in gzip format\n", COMPRESSED_FLAG_SHORT, COMPRESSED_FLAG);

  printf("  %s, %-16s low-latency output\n", INTERACTIVE_FLAG_SHORT, INTERACTIVE_FLAG);

  printf("  %s, %-16s markup translation errors in namespace \"%s\"\n",
	  DEBUG_FLAG_SHORT, DEBUG_FLAG, SRCML_ERR_NS_URI);

  printf("  %s, %-16s verbose output to standard error\n\n", VERBOSE_FLAG_SHORT, VERBOSE_FLAG);

  printf("Metadata Options:\n\n"
	 "  %s, %-16s set the language to %s, %s (default), %s, or %s\n",
	 LANGUAGE_FLAG_SHORT, LANGUAGE_FLAG_FULL,
	 LANGUAGE_C, LANGUAGE_CXX, LANGUAGE_JAVA, LANGUAGE_ASPECTJ);

  printf("  %s, %-16s set the directory attribute to DIR\n",
	 DIRECTORY_FLAG_SHORT, DIRECTORY_FLAG_FULL);

  printf("  %s, %-16s set the filename attribute to FILE\n",
	 FILENAME_FLAG_SHORT, FILENAME_FLAG_FULL);

  printf("  %s, %-16s set the version attribute to VER\n\n",
	 SRCVERSION_FLAG_SHORT, SRCVERSION_FLAG_FULL);

      /*
	      << "  " << SELF_VERSION_FLAG <<  "  " << "        "
	      << "set the version attribute to the src2srcml version\n"
      */

  printf("Markup Extensions:\n\n");

  printf("  %-16s markup literal values in namespace \"%s\"\n",
	 LITERAL_FLAG, SRCML_EXT_LITERAL_NS_URI);

  printf("  %-16s markup operators in namespace \"%s\"\n",
	 OPERATOR_FLAG, SRCML_EXT_OPERATOR_NS_URI);

  printf("  %-16s markup type modifiers in namespace \"%s\"\n",
	 MODIFIER_FLAG, SRCML_EXT_MODIFIER_NS_URI);

  printf("\nCPP Markup Options:\n\n");

  printf("  %-16s  markup cpp #else regions (default)\n", CPP_MARKUP_ELSE_FLAG);
  printf("  %-16s  leave cpp #else regions as text\n\n", CPP_TEXTONLY_ELSE_FLAG);

  printf("  %-16s  markup cpp #if 0 regions\n", CPP_MARKUP_IF0_FLAG);
  printf("  %-16s  leave cpp #if 0 regions as text (default)\n\n", CPP_TEXTONLY_IF0_FLAG);

  printf("Examples:  \n"
	 "  %1$s                (read from standard input, write to standard output)\n"
	 "  %1$s m.cpp          (read from file m.cpp, write to standard output)\n"
	 "  %1$s m.cpp -o m.cpp.xml (read from file m.cpp, write to file m.cpp.xml)\n"
	 "  %1$s -              (read from standard input, write to standard output)\n"
	 "  %1$s - -o m.cpp.xml    (read from standard input, write to file m.cpp.xml)\n"
	 "  %1$s --directory=src --filename=m.cpp - m.cpp.xml "
	 "(element unit attributes dir \"src\", filename \"m.cpp\")\n"
	 "  %1$s --src-encoding=UTF-8 m.cpp m.cpp.xml         "
	 "(encoding of input text file is UTF-8)\n"
	 "  %1$s --xml-encoding=ISO-8859-1 m.cpp m.cpp.xml    "
	 "(set encoding of srcML file to ISO-8859-1)\n\n", name);

  printf("www.sdml.info\n"
	 "Report bugs to %s\n", EMAIL_ADDRESS);
}

// output version message
void output_version(const char* name) {
  printf("%s Version %s\n%s\n", name, VERSION,COPYRIGHT);
}

int options = OPTION_CPP_MARKUP_ELSE | OPTION_CPP | OPTION_XMLDECL | OPTION_NAMESPACEDECL;
const char* src_encoding = DEFAULT_TEXT_ENCODING;
int language = 0;
const char* xml_encoding = DEFAULT_XML_ENCODING;
const char* given_directory = 0;
const char* given_filename = 0;
const char* fname = "-";
const char* given_version = 0;
bool specified_cpp_option = false;

// output filename
const char* srcml_filename = 0;

const char* num2prefix[] = {

  SRCML_SRC_NS_PREFIX_DEFAULT,
  SRCML_CPP_NS_PREFIX_DEFAULT,
  SRCML_ERR_NS_PREFIX_DEFAULT,
  SRCML_EXT_LITERAL_NS_PREFIX_DEFAULT,
  SRCML_EXT_OPERATOR_NS_PREFIX_DEFAULT,
  SRCML_EXT_MODIFIER_NS_PREFIX_DEFAULT,
};

bool prefixchange[] = {
  false,
  false,
  false,
  false,
  false,
  false,
};

// setup options and collect info from arguments
int process_args(int argc, char* argv[]);

#ifdef __GNUG__
extern "C" void verbose_handler(int);

extern "C" void terminate_handler(int);
#endif

int main(int argc, char* argv[]) {

  int exit_status = EXIT_SUCCESS;

  /* signal handling */

#ifdef __GNUG__
  // signal to toggle verbose flag
  pstd::signal(SIGUSR1, verbose_handler);
#endif

  // process command-line arguments
  int curarg = process_args(argc, argv);

  // help flag trumps all other options
  if (isoption(options, OPTION_HELP)) {
    output_help(NAME);
    exit(STATUS_SUCCESS);
  }

  // version flag trumps all other options except for help
  if (isoption(options, OPTION_PVERSION)) {
    output_version(NAME);
    exit(STATUS_SUCCESS);
  }

  /* Special checks for illegal combinations */


  // first command line parameter after options are the input filenames
  int input_arg_start = curarg;
  int input_arg_end = -1;
  int input_arg_count = 0;
  int input_arg_skip_start = -1;
  int input_arg_skip_end = -1;
  int numout = srcml_filename ? 0 : 1;
  if (argc - curarg == 1)
    numout = 0;
  while ((argc - curarg) > numout) {

    // mark last input filename assuming output srcml filename is last
    input_arg_end = curarg;

    // calculate the total number of input files
    ++input_arg_count;
    //    input_arg_count = input_arg_end - input_arg_start + 1;

    // update the argument count with the input filenames
    //    curarg += input_arg_count;
    ++curarg;
  }

  // last command line parameter is output srcml filename
  if (!srcml_filename) {
    srcml_filename = "-";
    if ((argc - (curarg - 1)) > 1) {
      srcml_filename = argv[curarg];

      ++curarg;
    }
  }

  // if more than one input filename assume nested
  if (input_arg_count > 1)
    options |= OPTION_NESTED;

  // verify that the output filename is not the same as any of the input filenames
#ifdef __GNUG__
  struct stat outstat;
  stat(srcml_filename, &outstat);
  for (int i = input_arg_start; i <= input_arg_end; ++i) {

    if (i >= input_arg_skip_start && i <= input_arg_skip_end)
      continue;

    struct stat instat;
    stat(argv[i], &instat);
    if (instat.st_ino == outstat.st_ino && instat.st_dev == outstat.st_dev) {
      fprintf(stderr, "%s: Input file '%s' is the same as the output file '%s'\n",
	      argv[0], argv[i], srcml_filename);
      exit(STATUS_INPUTFILE_PROBLEM);
    }
  }
#endif

  // make sure user did not specify duplicate prefixes as an option
  for (int i = 0; i < 6 - 1; ++i) {
    for (int j = i + 1; j < 6; ++j)
      if(strcmp(num2prefix[i], num2prefix[j]) == 0) {

	fprintf(stderr, "%s: Namespace conflict for ", NAME);
	if (num2prefix[i] == '\0') {
	  fprintf(stderr, "default prefix\n");
	} else {
	  fprintf(stderr, "prefix \'%s\'\n", num2prefix[i]);
	}
	fprintf(stderr, "Prefix URI conflicts:\n  %s\n  %s\n", num2uri[i], num2uri[j]);

	exit(STATUS_INVALID_OPTION_COMBINATION);
      }
  }

  // automatic interactive use from stdin (not on redirect or pipe)
#ifdef __GNUG__
  if (input_arg_count == 0 || strcmp(argv[input_arg_start], STDIN) == 0) {

    if (isatty(STDIN_FILENO))
      options |= OPTION_INTERACTIVE;
  }
#endif

  try {

    // for single file specified on command line, grab the language from the extension
    if (language == 0 && input_arg_count == 1) {

      char* path = argv[input_arg_start];
      char* ext = filename_ext(path);
      language = Language::getLanguageFromExtension(ext);

    }

    // turnoff default cpp reference for Java-based languages
    if (!specified_cpp_option && (language == srcMLTranslator::LANGUAGE_JAVA || language == srcMLTranslator::LANGUAGE_ASPECTJ))
	options &= ~OPTION_CPP;

    // translator from input to output using determined language
    //    if (language == 0)
    //	language = DEFAULT_LANGUAGE;
    srcMLTranslator translator(language == 0 ? DEFAULT_LANGUAGE : language, src_encoding, xml_encoding, srcml_filename, options, given_directory, given_filename, given_version, num2prefix);

  // output source encoding
  if (isoption(options, OPTION_VERBOSE)) {
    fprintf(stderr, "Source encoding:  %s\n", src_encoding);
    fprintf(stderr, "XML encoding:  %s\n", xml_encoding);
  }

  // translate input filenames from list in file
  if (isoption(options, OPTION_FILELIST)) {

#ifdef __GNUG__
    // setup so we can gracefully stop after a file at a time
    pstd::signal(SIGINT, terminate_handler);
#endif
      
    try {

      // translate all the filenames listed in the named file
      // Use libxml2 routines so that we can handle http:, file:, and gzipped files automagically
      URIStream uriinput(fname);
      int count = 0;
      char* line;
      while ((line = uriinput.getline())) {

	// skip blank lines or comment lines
	if (line[0] == '\0' || line[0] == FILELIST_COMMENT)
	  continue;

	// another file
	++count;

	// in verbose mode output the currently processed filename
	if (isoption(options, OPTION_VERBOSE))
	  fprintf(stderr, "%d\t%s", count, line);

	// turnoff default cpp reference for Java-based languages
	if (!specified_cpp_option && (language == srcMLTranslator::LANGUAGE_JAVA || language == srcMLTranslator::LANGUAGE_ASPECTJ))
	  options &= ~OPTION_CPP;

	// translate the file listed in the input file using the directory and filename extracted from the path
	char* dir = 0;
	char* filename = 0;
	try {
	  translator.setupInput(line);

	  // language based on extension
	  if (language == 0) {
	    char* ext = filename_ext(line);
	    language = Language::getLanguageFromExtension(ext);
	  }

	  filename_split(line, dir, filename);
	  translator.translate(dir,
			       filename,
			       given_version,
			       language);
	} catch (FileError) {

	  if (dir)
	    fprintf(stderr, isoption(options, OPTION_VERBOSE) ? "\t\terror: file \'%s/%s\' does not exist.\n" :
				   " error: file \'%s/%s\' does not exist.\n", dir, filename);
	  else
	    fprintf(stderr, isoption(options, OPTION_VERBOSE) ? "\t\terror: file \'%s\' does not exist.\n" :
				   " error: file \'%s\' does not exist.\n", filename);
	}

	if (isoption(options, OPTION_VERBOSE)) {
	  fprintf(stderr, "\n");
	}

	// compound documents are interrupted gracefully
	if (isoption(options, OPTION_TERMINATE))
	  return STATUS_TERMINATED;
      }

    } catch (URIStreamFileError) {
      fprintf(stderr, "%s error: file/URI \'%s\' does not exist.\n", argv[0], fname);
      exit(STATUS_INPUTFILE_PROBLEM);
    }

    // translate from standard input
  } else if (input_arg_count == 0 || strcmp(argv[input_arg_start], STDIN) == 0) {

    // translate from standard input using any directory, filename and version given on the command line
    translator.setupInput(STDIN);
    translator.translate(given_directory, given_filename, given_version, 
			 language ? language : DEFAULT_LANGUAGE);

  // translate single input filename from command line
  }  else if (input_arg_count == 1) {

    // translate from path given on command line using directory given on the command line or extracted
    // from full path
    char* path = argv[input_arg_start];
    translator.setupInput(path);
    char* path_s = 0;
    char* filename_s = 0;
    filename_split(path, path_s, filename_s);

    // hack to fix where directory, but no filename
    if (path_s && !filename_s[0]) {
      char* p = path_s;
      path_s = filename_s;
      filename_s = p;
    }

    try {
      translator.translate(isoption(options, OPTION_DIRECTORY) ? given_directory : path_s,
			   isoption(options, OPTION_FILENAME)  ? given_filename  : filename_s,
			   given_version,
			   language ? language : DEFAULT_LANGUAGE);

    } catch (FileError) {

      if (path_s)
	fprintf(stderr, "%s error: file \'%s/%s\' does not exist.\n", argv[0], path_s, filename_s);
      else
	fprintf(stderr, "%s error: file \'%s\' does not exist.\n", argv[0], filename_s);

      exit(STATUS_INPUTFILE_PROBLEM);
    }

  // translate multiple input filenames on command line
  } else {

    int count = 0;    // keep count for verbose mode

#ifdef __GNUG__
    // setup so we can gracefully stop after a file at a time
    pstd::signal(SIGINT, terminate_handler);
#endif
      
    // translate in batch the input files on the command line extracting the directory and filename attributes
    // from the full path
    for (int i = input_arg_start; i <= input_arg_end; ++i) {

      if (i >= input_arg_skip_start && i <= input_arg_skip_end)
	continue;

      char* path = argv[i];

      // another file
      ++count;

      // in verbose mode output the currently processed filename
      if (isoption(options, OPTION_VERBOSE)) {
	fprintf(stderr, "%d\t%s", count, path);
      }
      char* path_s = 0;
      char* filename_s = 0;
      try {
	translator.setupInput(path);
	filename_split(path, path_s, filename_s);
	translator.translate(path_s, filename_s, 0, language ? language : DEFAULT_LANGUAGE);
      } catch (FileError) {
	if (path_s)
	  fprintf(stderr, "%s error: file \'%s/%s\' does not exist.\n", argv[0], path_s, filename_s);
	else
	  fprintf(stderr, "%s error: file \'%s\' does not exist.\n", argv[0], filename_s);
      }

      if (isoption(options, OPTION_VERBOSE)) {
	fprintf(stderr, "\n");
      }

      // compound documents are interrupted gracefully
      if (isoption(options, OPTION_TERMINATE))
	return STATUS_TERMINATED;
    }
  }
  } catch (srcEncodingException) {
    fprintf(stderr, "Translation encoding problem\n");
    exit(STATUS_UNKNOWN_ENCODING);
  }

  return exit_status;
}

// setup options and collect info from arguments
int process_args(int argc, char* argv[]) {

  bool cpp_if0 = false;
  bool cpp_else = false;

  int curoption = 0;
  struct option cliargs[] = {
    { HELP_FLAG + 2, no_argument, NULL, HELP_FLAG_SHORT[1] },
    { VERSION_FLAG + 2, no_argument, NULL, VERSION_FLAG_SHORT[1] },
    { OUTPUT_FLAG + 2, required_argument, NULL, OUTPUT_FLAG_SHORT[1] },
    { FILELIST_FLAG + 2, required_argument, NULL, 'F' },
    { NESTED_FLAG + 2, no_argument, NULL, NESTED_FLAG_SHORT[1] },
    { EXPRESSION_MODE_FLAG + 2, no_argument, NULL, EXPRESSION_MODE_FLAG_SHORT[1] },
    { ENCODING_FLAG + 2, required_argument, NULL, ENCODING_FLAG_SHORT[1] },
    { TEXTENCODING_FLAG + 2, required_argument, NULL, TEXTENCODING_FLAG_SHORT[1] },
    { XMLNS_FLAG + 2, required_argument, NULL, 'X' },
    { COMPRESSED_FLAG + 2, no_argument, NULL, COMPRESSED_FLAG_SHORT[1] },
    { INTERACTIVE_FLAG + 2, no_argument, NULL, INTERACTIVE_FLAG_SHORT[1] },
    { DEBUG_FLAG + 2, no_argument, NULL, DEBUG_FLAG_SHORT[1] },
    { VERBOSE_FLAG + 2, no_argument, NULL, VERBOSE_FLAG_SHORT[1] },
    { LANGUAGE_FLAG + 2, required_argument, NULL, LANGUAGE_FLAG_SHORT[1] },
    { DIRECTORY_FLAG + 2, required_argument, NULL, DIRECTORY_FLAG_SHORT[1] },
    { FILENAME_FLAG + 2, required_argument, NULL, FILENAME_FLAG_SHORT[1] },
    { SRCVERSION_FLAG + 2, required_argument, NULL, SRCVERSION_FLAG_SHORT[1] },
    { LITERAL_FLAG + 2, required_argument, NULL, 'T' },
    { OPERATOR_FLAG + 2, required_argument, NULL, 'O' },
    { MODIFIER_FLAG + 2, required_argument, NULL, 'M' },
    { CPP_MARKUP_ELSE_FLAG + 2, no_argument, NULL, 'm' },
    { CPP_TEXTONLY_ELSE_FLAG + 2, no_argument, NULL, 'E' },
    { CPP_MARKUP_IF0_FLAG + 2, no_argument, NULL, '0' },
    { CPP_TEXTONLY_IF0_FLAG + 2, no_argument, NULL, 'p' },
    { NO_XML_DECLARATION_FLAG + 2, no_argument, &curoption, OPTION_XMLDECL },
    { NO_NAMESPACE_DECLARATION_FLAG + 2, no_argument, &curoption, OPTION_NAMESPACEDECL },
    { 0, 0, 0, 0 }
  };

  // process all command line options
  int position = 0;
  int curarg = 1;  // current argument
  char* embedded;
  const char* ns_prefix;
  const char* ns_uri;

  while (1) {
    curoption = 0;
    int option_index = 0;
    bool special = optind < argc && !strncmp(argv[optind], "--xmlns:", 8);
    opterr = !special ? 1 : 0;
    int c = getopt_long(argc, argv, "hVo:F:nex:t:X:zcgvl:d:f:s:TOMmE0p", cliargs, &option_index);

    if (c == -1)
      break;

    if (curoption) {
      options &= ~curoption;
      continue;
    }

    // treat --xmlns:prefix=url as --xmlns=url for processing
    if (special && c == '?') {
      c = 'X';
    }

    // missing or extra option argument
    if (c == '?') {
      fprintf(stderr, "Try '%s %s' for more information.\n", argv[0], HELP_FLAG);
      exit(option_error_status(optopt));
    }

    char* end = 0;
    switch(c) {

    case 'h': 
      options |= OPTION_HELP;
      break;

    case 'V': 
      options |= OPTION_PVERSION;
      break;

    case 'o': 
      srcml_filename = optarg;
      break;

    case 'F': 
      options |= OPTION_FILELIST;

      // filelist mode is default nested mode
      options |= OPTION_NESTED;

      fname = optarg;
      break;

    case 'n': 
      options |= OPTION_NESTED;
      break;

    case 'e': 
      options |= OPTION_EXPRESSION;
      break;

    case 'x': 
      options |= OPTION_XML_ENCODING;

      xml_encoding = optarg;

      // validate xml encoding
      if (!srcMLOutput::checkEncoding(xml_encoding)) {
	fprintf(stderr, "%s: xml encoding \"%s\" is not supported.\n", argv[0], xml_encoding);
	exit(STATUS_UNKNOWN_ENCODING);
      }
      break;

    case 't': 
      options |= OPTION_TEXT_ENCODING;

      src_encoding = optarg;

      // validate source encoding
      if (!srcMLOutput::checkEncoding(src_encoding)) {
	fprintf(stderr, "%s: text encoding \"%s\" is not supported.\n", argv[0], src_encoding);
	exit(STATUS_UNKNOWN_ENCODING);
      }
      break;

    case 'X': 
      curarg = optind - 1;
      embedded = extract_option(argv[curarg]);

      // filename is embedded parameter
      if (embedded) {

	if (argv[curarg][strlen(XMLNS_FLAG)] != ':')
	  ns_prefix = "";
	else {
	  *embedded = '\0';
	  ns_prefix = argv[curarg] + strlen(XMLNS_FLAG) + 1;
	}

	ns_uri = embedded + 1;
	
      // check for language flag with missing language value
      } else if (argc <= curarg + 1 || strcmp(argv[curarg + 1], OPTION_SEPARATOR) == 0) {
	fprintf(stderr, "%s: xmlns option selected but not specified.\n", NAME);
	exit(STATUS_LANGUAGE_MISSING);
      } else {

	// extract prefix
	if (strlen(argv[curarg]) == strlen(XMLNS_FLAG))
	  ns_prefix = "";
	else
	  ns_prefix = argv[curarg] + strlen(XMLNS_FLAG) + 1;

	// uri is next argument
	ns_uri = argv[++curarg];
      }

      ++curarg;

      // update the uri's
      // check for standard namespaces, store them, and update any flags
      if (strcmp(ns_uri, SRCML_SRC_NS_URI) == 0) {

	num2prefix[SRCML_SRC_NS_URI_POS] = ns_prefix;
	prefixchange[SRCML_SRC_NS_URI_POS] = true;

      } else if (strcmp(ns_uri, SRCML_CPP_NS_URI) == 0) {

	// specifying the cpp prefix automatically turns on preprocessor
	options |= OPTION_CPP;
	specified_cpp_option = true;

	num2prefix[SRCML_CPP_NS_URI_POS] = ns_prefix;
	prefixchange[SRCML_CPP_NS_URI_POS] = true;

      } else if (strcmp(ns_uri, SRCML_ERR_NS_URI) == 0) {

	// specifying the error prefix automatically turns on debugging
	options |= OPTION_DEBUG;

	num2prefix[SRCML_ERR_NS_URI_POS] = ns_prefix;
	prefixchange[SRCML_ERR_NS_URI_POS] = true;

      } else if (strcmp(ns_uri, SRCML_EXT_LITERAL_NS_URI) == 0) {

	// specifying the literal prefix automatically turns on literal markup
	options |= OPTION_LITERAL;

	num2prefix[SRCML_EXT_LITERAL_NS_URI_POS] = ns_prefix;
	prefixchange[SRCML_EXT_LITERAL_NS_URI_POS] = true;

      } else if (strcmp(ns_uri, SRCML_EXT_OPERATOR_NS_URI) == 0) {

	// specifying the operator prefix automatically turns on operator markup
	options |= OPTION_OPERATOR;

	num2prefix[SRCML_EXT_OPERATOR_NS_URI_POS] = ns_prefix;
	prefixchange[SRCML_EXT_OPERATOR_NS_URI_POS] = true;

      } else if (strcmp(ns_uri, SRCML_EXT_MODIFIER_NS_URI) == 0) {

	// specifying the operator prefix automatically turns on type modifier markup
	options |= OPTION_MODIFIER;

	num2prefix[SRCML_EXT_MODIFIER_NS_URI_POS] = ns_prefix;
	prefixchange[SRCML_EXT_MODIFIER_NS_URI_POS] = true;

      } else {
	fprintf(stderr, "%s: invalid namespace \"%s\"\n\n"
		"Namespace URI must be on of the following:  \n"
		"  %-35s primary srcML namespace\n"
		"  %-35s namespace for cpreprocessing elements\n"
		"  %-35s namespace for srcML debugging elements\n"
		"  %-35s namespace for optional literal elements\n"
		"  %-35s namespace for optional operator element\n"
		"  %-35s namespace for optional modifier element\n",
		argv[0], ns_uri,
		SRCML_SRC_NS_URI, SRCML_CPP_NS_URI, SRCML_ERR_NS_URI,
		SRCML_EXT_LITERAL_NS_URI, SRCML_EXT_OPERATOR_NS_URI, SRCML_EXT_MODIFIER_NS_URI
		);
	exit(STATUS_INVALID_LANGUAGE);
      }
      //      fprintf(stderr, "XMLNS: %s\n", optarg);
      break;

    case 'z': 
      options |= OPTION_COMPRESSED;
      break;

    case 'c': 
      options |= OPTION_INTERACTIVE;
      break;

    case 'g': 
      options |= OPTION_DEBUG;
      break;

    case 'v': 
      options |= OPTION_VERBOSE;
      break;

    case 'l': 
      options |= OPTION_LANGUAGE;

      // validate language selected
      language = Language::getLanguage(optarg);
      if (language == 0) {
	fprintf(stderr, "%s: invalid option -- Language flag must one of the following values:  "
		"%s %s %s %s\n", argv[0], LANGUAGE_C, LANGUAGE_CXX, LANGUAGE_JAVA, LANGUAGE_ASPECTJ);

	exit(STATUS_INVALID_LANGUAGE);
      }
      break;

    case 'd': 
      options |= OPTION_DIRECTORY;

      given_directory = optarg;
      break;

    case 'f': 
      options |= OPTION_FILENAME;

      given_filename = optarg;
      break;

    case 's': 
      options |= OPTION_VERSION;

      given_version = optarg;
      break;

    case 'T': 
      options |= OPTION_LITERAL;
      break;

    case 'O': 
      options |= OPTION_OPERATOR;
      break;

    case 'M': 
      options |= OPTION_MODIFIER;
      break;

    case 'm': 
      options |= OPTION_CPP_MARKUP_ELSE;
      break;

    case 'E': 
	options &= ~OPTION_CPP_MARKUP_ELSE;
      break;

    case '0': 
	options |= OPTION_CPP_MARKUP_IF0;
      break;

    case 'p': 
	options &= ~OPTION_CPP_MARKUP_IF0;
      break;

    default:
      exit(STATUS_UNKNOWN_OPTION);
      break;
    };
  }
  /*
    // markup of cpp #else mode
    else if (compare_flags(argv[curarg], CPP_MARKUP_ELSE_FLAG, CPP_MARKUP_ELSE_FLAG_SHORT, position)) {
      if (!cpp_else) {
	options |= OPTION_CPP_MARKUP_ELSE;
	if (position == original_position) ++curarg;

	cpp_else = true;

      } else {
	fprintf(stderr, "%s: Conflicting options %s and %s selected.\n",
		argv[0], CPP_MARKUP_ELSE_FLAG, CPP_TEXTONLY_ELSE_FLAG);
	exit(STATUS_INVALID_OPTION_COMBINATION);
      }
    }

    // text-only cpp #else mode
    else if (compare_flags(argv[curarg], CPP_TEXTONLY_ELSE_FLAG, CPP_TEXTONLY_ELSE_FLAG_SHORT, position)) {

      if (!cpp_else) {
	options &= ~OPTION_CPP_MARKUP_ELSE;
	if (position == original_position) ++curarg;

	cpp_else = true;
      } else {
	fprintf(stderr, "%s: Conflicting options %s and %s selected.\n",
		argv[0], CPP_MARKUP_ELSE_FLAG, CPP_TEXTONLY_ELSE_FLAG);
	exit(STATUS_INVALID_OPTION_COMBINATION);
      }
    }

    // markup of cpp #if 0 mode
    else if (compare_flags(argv[curarg], CPP_MARKUP_IF0_FLAG, CPP_MARKUP_IF0_FLAG_SHORT, position)) {
      if (!cpp_if0) {
	options |= OPTION_CPP_MARKUP_IF0;
	if (position == original_position) ++curarg;

	cpp_if0 = true;

      } else {
	fprintf(stderr, "%s: Conflicting options %s and %s selected.\n",
		argv[0], CPP_MARKUP_IF0_FLAG, CPP_TEXTONLY_IF0_FLAG);
	exit(STATUS_INVALID_OPTION_COMBINATION);
      }
    }

    // text-only cpp #if 0 mode
    else if (compare_flags(argv[curarg], CPP_TEXTONLY_IF0_FLAG, CPP_TEXTONLY_IF0_FLAG_SHORT, position)) {

      if (!cpp_if0) {
	// clear if previously marked
	options &= ~OPTION_CPP_MARKUP_IF0;
	if (position == original_position) ++curarg;

	cpp_if0 = true;
      } else {
	fprintf(stderr, "%s: Conflicting options %s and %s selected.\n",
		argv[0], CPP_MARKUP_IF0_FLAG, CPP_TEXTONLY_IF0_FLAG);
	exit(STATUS_INVALID_OPTION_COMBINATION);
      }
    }
  */
  return optind;
}

#ifdef __GNUG__
extern "C" void verbose_handler(int) {

  if ((options &= OPTION_VERBOSE) == 0)
    options |= OPTION_VERBOSE;
  else
    options &= ~OPTION_VERBOSE;
}

extern "C" void terminate_handler(int) {

  // setup to terminate after current file
  options |= OPTION_TERMINATE;

  // turn off handler for this signal
  pstd::signal(SIGINT, SIG_DFL);
}
#endif

int option_error_status(int optopt) {

  switch (optopt) {

  case 'f':
    return STATUS_FILENAME_MISSING;
    break;

  case 'l':
    return STATUS_LANGUAGE_MISSING;
    break;

  case 'd':
    return STATUS_DIRECTORY_MISSING;
    break;

  case 's':
    return STATUS_VERSION_MISSING;
    break;

  case 'x':
    return STATUS_XMLENCODING_MISSING;
    break;

  case 't':
    return STATUS_SRCENCODING_MISSING;
    break;

  case 'U':
    return STATUS_UNIT_MISSING;
    break;

  case '\0':
    return STATUS_UNKNOWN_OPTION;
    break;
  };

  return 0;
}
