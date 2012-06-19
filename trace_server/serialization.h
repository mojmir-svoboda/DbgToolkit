#pragma once

class SessionState;

bool saveSessionState (SessionState const & s, char const * filename);
bool loadSessionState (SessionState & s, char const * filename);

bool loadSessionState (SessionState const & src, SessionState & target);
