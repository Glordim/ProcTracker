cmake_minimum_required(VERSION 3.4...3.27)

function(EmbeedTextInSource InputFile TemplateFile OutputDir OutputFile)

	get_filename_component(FILENAME "${InputFile}" NAME)
	message(STATUS "EmbeedBinaryInSource: ${InputFile} -> ${OutputDir}/${FILENAME}.hpp")

	string(REGEX REPLACE "\\.| |-" "_" ESCAPED_FILE_NAME ${FILENAME})

	file(READ "${InputFile}" FILE_CONTENTS)
	string(REPLACE "\"" "\\\"" ESCAPED_FILE_CONTENTS "${FILE_CONTENTS}")
	string(REPLACE "\n" "\\n\"\n\"" ESCAPED_FILE_CONTENTS "${ESCAPED_FILE_CONTENTS}")

	set(${OutputFileH} ${OutputDir}/${FILENAME}.hpp PARENT_SCOPE)
	configure_file(
		${TemplateFile}
		${OutputFileH}
		@ONLY
	)
endfunction()

function(EmbeedBinaryInSource InputFile TemplateFileH TemplateFileCPP OutputDir OutputFileH OutputFileCPP)

	get_filename_component(FILENAME "${InputFile}" NAME)
	message(STATUS "EmbeedBinaryInSource: ${InputFile} -> ${OutputDir}/${FILENAME}.cpp and ${OutputDir}/${FILENAME}.hpp")

	string(REGEX REPLACE "\\.| |-" "_" ESCAPED_FILE_NAME ${FILENAME})

	file(READ "${InputFile}" FILE_CONTENTS HEX)
	string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," ESCAPED_FILE_CONTENTS ${FILE_CONTENTS}) # Convert hex data for C compatibility

	set(${OutputFileH} ${OutputDir}/${FILENAME}.hpp PARENT_SCOPE)
	configure_file(
		${TemplateFileH}
		${OutputFileH}
		@ONLY
	)
	set(${IncludesVar} ${Includes} PARENT_SCOPE)

	set(${OutputFileCPP} ${OutputDir}/${FILENAME}.cpp PARENT_SCOPE)
	configure_file(
		${TemplateFileCPP}
		${OutputFileCPP}
		@ONLY
	)
endfunction()
