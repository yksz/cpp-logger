#pragma once

namespace logger {

/**
 * Configure a logger with a configuration file.
 * If the filename is nullptr, return without doing anything.
 *
 * The following is the configurable key/value list.
 * |key                        |value                                        |
 * |:--------------------------|:--------------------------------------------|
 * |level                      |TRACE, DEBUG, INFO, WARN, ERROR or FATAL     |
 * |logger                     |console or file                              |
 * |logger.console.output      |stdout or stderr                             |
 * |logger.file.filename       |A output filename                            |
 *
 * @param[in] filename The name of the configuration file
 * @return true upon success or false on error
 */
bool Configure(const char* filename);

} // namespace logger
