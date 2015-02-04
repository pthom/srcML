#!/bin/bash

# test framework
source $(dirname "$0")/framework_test.sh

define srcmla <<- 'STDOUT'
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	<unit xmlns="http://www.sdml.info/srcML/src" xmlns:cpp="http://www.sdml.info/srcML/cpp" revision="0.8.0" language="C++" filename="a.cpp" hash="a301d91aac4aa1ab4e69cbc59cde4b4fff32f2b8"><expr_stmt><expr><name>a</name></expr>;</expr_stmt></unit>
	STDOUT

define srcmlb <<- 'STDOUT'
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	<unit xmlns="http://www.sdml.info/srcML/src" xmlns:cpp="http://www.sdml.info/srcML/cpp" revision="0.8.0" language="C++" filename="b.cpp" hash="9a1e1d3d0e27715d29bcfbf72b891b3ece985b36"><expr_stmt><expr><name>b</name></expr>;</expr_stmt></unit>
	STDOUT

# test --no-archive
createfile a.cpp "a;"
createfile b.cpp "b;"

srcml2src --no-archive --to-dir . a.cpp b.cpp
check a.cpp.xml 3<<< "$srcmla"
check b.cpp.xml 3<<< "$srcmlb"

srcml2src --to-dir . --no-archive a.cpp b.cpp
check a.cpp.xml 3<<< "$srcmla"

srcml2src --no-archive a.cpp b.cpp --to-dir .
check a.cpp.xml 3<<< "$srcmla"

srcml2src a.cpp b.cpp --no-archive --to-dir .
check a.cpp.xml 3<<< "$srcmla"

srcml2src a.cpp b.cpp --to-dir . --no-archive
check a.cpp.xml 3<<< "$srcmla"
