#pragma once

namespace trace {

	/**@fn		compareFiles
	 * @brief	compares two binary files and returns the result
	 * @return	true if the two files are equal, false otherwise
	 */
	bool compareFiles (char const * origname, char const * runname);

	/**@fn		tryUpdateTraceServer
	 * @brief	tries to update trace server
	 * @return	true if trace server needs to be updated
	 *			false otherwise
	 */
	bool tryUpdateTraceServer (char const * origname, char const * runname);

	/**@fn		tryCopyTraceServer
	 * @brief	tries to copy trace server from origname to runname
	 * @return	true if success
	 *			false otherwise
	 */
	bool tryCopyTraceServer (char const * origname, char const * runname);

	void runTraceServer (char const * runname, char const * args);

} // namespace

