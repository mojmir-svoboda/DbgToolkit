#pragma once

namespace trace {
	typedef uint32_t context_t;
}

const trace::context_t CTX_Init				=  (1 <<  0);
const trace::context_t CTX_Main					=  (1 <<  1);
const trace::context_t CTX_Render      =  (1 <<  2);
const trace::context_t CTX_Other     =  (1 <<  3);
