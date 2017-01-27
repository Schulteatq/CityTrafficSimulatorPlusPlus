# Assigns the passed Files to source groups according to their file path.
# The path prefixes as well as dot operators are removed from the path.
# E.g.: path/to/my/file.cpp               =>  path\\to\\my
#       <PathPrefix>/path/to/my/file.cpp  =>  path\\to\\my
#       ../../path/to/my/file.cpp         =>  path\\to\\my
MACRO(DEFINE_SOURCE_GROUPS_FROM_SUBDIR Files BasePath RemovePathPrefixes)
    FOREACH(f ${${Files}})
        SET(f_cat "")
        
        # extract relative file path
        IF(IS_ABSOLUTE ${f})
            FILE(RELATIVE_PATH f_rel ${BasePath} ${f})
            GET_FILENAME_COMPONENT(f_rel ${f_rel} PATH)
        ELSE()
            GET_FILENAME_COMPONENT(f_rel ${f} PATH)
        ENDIF()
        
        # create source group specifier from rel path
        IF(f_rel)
            # remove ../
            string(REGEX REPLACE "\\.\\./" "" f_cat ${f_rel})
            # remove specified prefixes
            FOREACH(prefix ${RemovePathPrefixes})
                IF(f_cat)
                    string(REGEX REPLACE "${prefix}" "" f_cat ${f_cat})
                ENDIF()
            ENDFOREACH()
			
            # convert path separators into source group separators: 
            # path/to/my => path\\to\\my
            IF(f_cat)
                string(REGEX REPLACE "\\\\|/" "\\\\\\\\" f_cat ${f_cat})
            ENDIF()
        ENDIF()
        
        # set extracted source group
        IF(f_cat)
            SOURCE_GROUP("${f_cat}" FILES ${f})
        ELSE()
            SOURCE_GROUP("" FILES ${f})
        ENDIF()
        
    ENDFOREACH()
ENDMACRO(DEFINE_SOURCE_GROUPS_FROM_SUBDIR)
