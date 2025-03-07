cmake_minimum_required(VERSION 3.4...3.27)

function(EmbedTextInSource InputFile TemplateFile OutputDir OutOutputFileH)

	get_filename_component(FILENAME "${InputFile}" NAME)
	set(OutputFileH "${OutputDir}/${FILENAME}.hpp")
	set(${OutOutputFileH} "${OutputFileH}" PARENT_SCOPE)

	message(STATUS "EmbedBinaryInSource: ${InputFile} -> ${OutputDir}/${FILENAME}.hpp")

	string(REGEX REPLACE "\\.| |-" "_" ESCAPED_FILE_NAME ${FILENAME})

	file(READ "${InputFile}" FILE_CONTENTS)
	string(REPLACE "\"" "\\\"" ESCAPED_FILE_CONTENTS "${FILE_CONTENTS}")
	string(REPLACE "\n" "\\n\"\n\"" ESCAPED_FILE_CONTENTS "${ESCAPED_FILE_CONTENTS}")

	configure_file(
		${TemplateFile}
		${OutputFileH}
		@ONLY
	)
endfunction()

function(EmbedBinaryInSource InputFile TemplateFileH TemplateFileCPP OutputDir OutOutputFileH OutOutputFileCPP)

	get_filename_component(FILENAME "${InputFile}" NAME)

	set(OutputFileH "${OutputDir}/${FILENAME}.hpp")
	set(OutputFileCPP "${OutputDir}/${FILENAME}.cpp")
	set(${OutOutputFileH} "${OutputFileH}" PARENT_SCOPE)
	set(${OutOutputFileCPP} "${OutputFileCPP}" PARENT_SCOPE)

	message(STATUS "EmbedBinaryInSource: ${InputFile} -> ${OutputFileCPP} and ${OutputFileH}")

	string(REGEX REPLACE "\\.| |-" "_" ESCAPED_FILE_NAME ${FILENAME})

	file(READ "${InputFile}" FILE_CONTENTS HEX)
	string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," ESCAPED_FILE_CONTENTS ${FILE_CONTENTS}) # Convert hex data for C compatibility

	configure_file(
		${TemplateFileH}
		${OutputFileH}
		@ONLY
	)

	configure_file(
		${TemplateFileCPP}
		${OutputFileCPP}
		@ONLY
	)
endfunction()
