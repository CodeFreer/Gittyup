set(CONF_DIR ${CMAKE_SOURCE_DIR}/conf)
set(RSRC_DIR ${CMAKE_SOURCE_DIR}/rsrc)

option(GENERATE_APPDATA "Create appdata/metadata file" OFF)

if(APPLE)
  set(ICON ${RSRC_DIR}/Gittyup.icns)
  set(APP_IMPL_FILE Application_mac.mm)
  set(THEME_IMPL_FILE CustomTheme_mac.mm)
elseif(WIN32)
  set(RC_FILE ${RSRC_DIR}/Gittyup.rc)
endif()

set(EMOJI ${CONF_DIR}/emoji.json)
set(MERGETOOLS ${CONF_DIR}/mergetools)

file(GLOB CONFIG_FILES ${CONF_DIR}/*.lua)
file(GLOB DICTIONARIES ${CONF_DIR}/dictionaries/*.aff
     ${CONF_DIR}/dictionaries/*.dic ${CONF_DIR}/dictionaries/*.txt)
file(GLOB THEMES ${CONF_DIR}/themes/*.lua)
file(GLOB SCINTILLUA_LEXERS ${SCINTILLUA_LEXERS_DIR}/*.lua)
file(GLOB LUA_PLUGINS ${CONF_DIR}/plugins/*.lua ${CONF_DIR}/plugins/*.txt)

# Build resources.
qt5_add_resources(RESOURCES ${RSRC_DIR}/resources.qrc)

# Generate acknowledgments.
set(DOC_SOURCE_DIR ${CMAKE_SOURCE_DIR}/docs)
set(DOC_BINARY_DIR ${CMAKE_BINARY_DIR}/docs)
set(CHANGELOG_MD ${DOC_SOURCE_DIR}/changelog.md)
set(ACKNOWLEDGMENTS_MD ${DOC_SOURCE_DIR}/acknowledgments.md)
set(PRIVACY_MD ${DOC_SOURCE_DIR}/privacy.md)
set(PLUGIN_MD ${DOC_SOURCE_DIR}/plugin.md)
set(CHANGELOG_HTML ${DOC_BINARY_DIR}/changelog.html)
set(ACKNOWLEDGMENTS_HTML ${DOC_BINARY_DIR}/acknowledgments.html)
set(PRIVACY_HTML ${DOC_BINARY_DIR}/privacy.html)
set(PLUGIN_HTML ${DOC_BINARY_DIR}/plugin.html)
set(APPDATA_CONF
    ${CMAKE_SOURCE_DIR}/rsrc/linux/com.github.Murmele.Gittyup.appdata.xml.in)
set(APPDATA
    ${CMAKE_BINARY_DIR}/rsrc/linux/com.github.Murmele.Gittyup.appdata.xml)
add_custom_command(
  OUTPUT ${CHANGELOG_HTML} ${ACKNOWLEDGMENTS_HTML} ${PRIVACY_HTML}
         ${PLUGIN_HTML}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${DOC_BINARY_DIR}
  COMMAND cmark_exe ${CHANGELOG_MD} > ${CHANGELOG_HTML}
  COMMAND cmark_exe ${ACKNOWLEDGMENTS_MD} > ${ACKNOWLEDGMENTS_HTML}
  COMMAND cmark_exe ${PRIVACY_MD} > ${PRIVACY_HTML}
  COMMAND cmark_exe ${PLUGIN_MD} > ${PLUGIN_HTML}
  DEPENDS ${CHANGELOG_MD} ${ACKNOWLEDGMENTS_MD} ${PRIVACY_MD} ${PLUGIN_MD})

# Add application library.
add_library(app Application.cpp CustomTheme.cpp Theme.cpp ${RESOURCES}
                ${APP_IMPL_FILE} ${THEME_IMPL_FILE})

if(UNIX AND NOT APPLE)
  set_target_properties(app PROPERTIES AUTOMOC ON)
endif()

target_compile_definitions(
  app
  PUBLIC GITTYUP_NAME="${GITTYUP_NAME}" GITTYUP_VERSION="${GITTYUP_VERSION}"
  PRIVATE QT_TRANSLATIONS_DIR="${QT_TRANSLATIONS_DIR}"
          BUILD_DESCRIPTION="${BUILD_DESCRIPTION}")

target_link_libraries(app conf git ui update Qt5::Widgets)

set_target_properties(app PROPERTIES AUTOMOC ON)

if(UNIX AND NOT APPLE)
  target_link_libraries(app Qt5::DBus)
endif()

# Add main executable.
add_executable(
  gittyup WIN32 MACOSX_BUNDLE
  Gittyup.cpp
  ${ICON}
  ${RC_FILE}
  ${CONFIG_FILES}
  ${DICTIONARIES}
  ${EMOJI}
  ${MERGETOOLS}
  ${THEMES}
  ${LUA_PLUGINS}
  ${SCINTILLUA_LEXERS}
  ${CHANGELOG_HTML}
  ${ACKNOWLEDGMENTS_HTML}
  ${PRIVACY_HTML}
  ${PLUGIN_HTML})

if(GENERATE_APPDATA)
  add_custom_target(
    appdata ALL
    COMMAND
      ${CMAKE_COMMAND} -DCHANGELOG_HTML:STRING=${CHANGELOG_HTML}
      -DAPPDATA_CONF:STRING=${APPDATA_CONF} -DAPPDATA:STRING=${APPDATA} -P
      "${CMAKE_SOURCE_DIR}/cmake/generate_appdata.cmake"
    DEPENDS ${CHANGELOG_HTML}
    COMMENT "Create appdata file")
endif()

target_link_libraries(gittyup app)

set_target_properties(
  gittyup PROPERTIES OUTPUT_NAME ${GITTYUP_NAME} RUNTIME_OUTPUT_DIRECTORY
                                                 ${CMAKE_BINARY_DIR})

if(WIN32)
  target_link_libraries(app Dbghelp.lib)
elseif(APPLE)
  set_target_properties(
    gittyup
    PROPERTIES INSTALL_RPATH "/usr/local/qt;@loader_path/../Frameworks"
               MACOSX_BUNDLE_INFO_PLIST ${RSRC_DIR}/Info.plist.in
               MACOSX_BUNDLE_ICON_FILE ${GITTYUP_NAME}
               MACOSX_BUNDLE_BUNDLE_NAME ${GITTYUP_NAME}
               MACOSX_BUNDLE_BUNDLE_VERSION ${GITTYUP_VERSION}
               MACOSX_BUNDLE_LONG_VERSION_STRING ${GITTYUP_VERSION}
               MACOSX_BUNDLE_SHORT_VERSION_STRING ${GITTYUP_VERSION}
               MACOSX_BUNDLE_GUI_IDENTIFIER "com.Murmele.Gittyup")

  # Copy config files into bundle.
  set_source_files_properties(
    ${ICON}
    ${EMOJI}
    ${MERGETOOLS}
    ${CONFIG_FILES}
    ${CHANGELOG_HTML}
    ${ACKNOWLEDGMENTS_HTML}
    ${PRIVACY_HTML}
    ${PLUGIN_HTML}
    PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

  set_source_files_properties(${DICTIONARIES} PROPERTIES MACOSX_PACKAGE_LOCATION
                                                         Resources/dictionaries)

  set_source_files_properties(${THEMES} PROPERTIES MACOSX_PACKAGE_LOCATION
                                                   Resources/themes)

  set_source_files_properties(${LUA_PLUGINS} PROPERTIES MACOSX_PACKAGE_LOCATION
                                                        Resources/plugins)

  set_source_files_properties(
    ${SCINTILLUA_LEXERS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/lexers)
else()

  if(NOT FLATPAK)
    set_target_properties(gittyup PROPERTIES INSTALL_RPATH "$ORIGIN")
  endif()
endif()

add_dependencies(gittyup indexer relauncher)

if(FLATPAK)
  # Install application.
  install(
    TARGETS gittyup
    DESTINATION ./bin # otherwise the executable will not be found by flatpak
    COMPONENT ${GITTYUP_NAME})
else()
  # Install application.
  install(
    TARGETS gittyup
    DESTINATION .
    COMPONENT ${GITTYUP_NAME})
endif()

if(APPLE)
  get_target_property(QT_LIBRARY Qt5::Core LOCATION)
  get_filename_component(QT_FRAMEWORK ${QT_LIBRARY} DIRECTORY)
  get_filename_component(RPATH ${QT_FRAMEWORK} DIRECTORY)

  # Fixup relauncher RPATH.
  set(RELAUNCHER ${GITTYUP_NAME}.app/Contents/MacOS/relauncher)
  install(
    CODE "execute_process(COMMAND
		${CMAKE_INSTALL_NAME_TOOL} -delete_rpath \"${RPATH}\"
	  \"\${CMAKE_INSTALL_PREFIX}/${RELAUNCHER}\"
  )
execute_process(COMMAND
	${CMAKE_INSTALL_NAME_TOOL} -add_rpath \"/usr/local/qt\"
	  \"\${CMAKE_INSTALL_PREFIX}/${RELAUNCHER}\"
  )
execute_process(COMMAND
	${CMAKE_INSTALL_NAME_TOOL} -add_rpath \"@loader_path/../Frameworks\"
	  \"\${CMAKE_INSTALL_PREFIX}/${RELAUNCHER}\"
  )"
    COMPONENT ${GITTYUP_NAME})

  # Fixup indexer RPATH.
  set(INDEXER ${GITTYUP_NAME}.app/Contents/MacOS/indexer)
  install(
    CODE "execute_process(COMMAND
		${CMAKE_INSTALL_NAME_TOOL} -delete_rpath \"${RPATH}\"
	  \"\${CMAKE_INSTALL_PREFIX}/${INDEXER}\"
  )
execute_process(COMMAND
	${CMAKE_INSTALL_NAME_TOOL} -add_rpath \"/usr/local/qt\"
	  \"\${CMAKE_INSTALL_PREFIX}/${INDEXER}\"
  )
execute_process(COMMAND
	${CMAKE_INSTALL_NAME_TOOL} -add_rpath \"@loader_path/../Frameworks\"
	  \"\${CMAKE_INSTALL_PREFIX}/${INDEXER}\"
  )"
    COMPONENT ${GITTYUP_NAME})
else()
  # Install config files.
  if(CHANGELOG_HTML
     AND ACKNOWLEDGMENTS_HTML
     AND PRIVACY_HTML
     AND PLUGIN_HTML)
    install(
      FILES ${CHANGELOG_HTML} ${ACKNOWLEDGMENTS_HTML} ${PRIVACY_HTML}
            ${PLUGIN_HTML}
      DESTINATION Resources
      COMPONENT ${GITTYUP_NAME})
  endif()

  install(
    FILES ${EMOJI} ${MERGETOOLS}
    DESTINATION Resources
    COMPONENT ${GITTYUP_NAME})

  foreach(CONFIG_FILE ${CONFIG_FILES})
    install(
      FILES ${CONFIG_FILE}
      DESTINATION Resources
      COMPONENT ${GITTYUP_NAME})
  endforeach()

  install(
    FILES ${DICTIONARIES}
    DESTINATION Resources/dictionaries
    COMPONENT ${GITTYUP_NAME})

  install(
    FILES ${THEMES}
    DESTINATION Resources/themes
    COMPONENT ${GITTYUP_NAME})

  install(
    FILES ${LUA_PLUGINS}
    DESTINATION Resources/plugins
    COMPONENT ${GITTYUP_NAME})

  foreach(SCINTILLUA_LEXER ${SCINTILLUA_LEXERS})
    install(
      FILES ${SCINTILLUA_LEXER}
      DESTINATION Resources/lexers
      COMPONENT ${GITTYUP_NAME})
  endforeach()
endif()
