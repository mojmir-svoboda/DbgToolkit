#pragma once

namespace trace {

	/**@fn		compareFiles
	 * @brief	compares two binary files and returns the result
	 * @return	true if the two files are equal, false otherwise
	 */
	bool compareFiles (char const * origname, char const * runname);

	void tryUpdateTraceServer (char const * origname, char const * runname);

	void runTraceServer (char const * origname, char const * runname);

} // namespace

