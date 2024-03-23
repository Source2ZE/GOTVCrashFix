# GOTVCrashFix

A Metamod plugin that fixes crashing induced by enabled gotv. This crash happens as a result of mismatched maxplayers between game session config and the rest of the game. The entity system would use the wrong maxplayers to reserve space for player controllers and thus when the last player filling up the server would join, it would kill the first new entity (most often cs_team_manager) and crash the entire server.

## Symptoms caused by this bug

- `HandleCommand_JoinTeam( xxxx ) - invalid team index.`
- Random entity being randomly killed (e.g. sky, team manager)
- strlen crash inside vsnprintf with `%s<%i><%s><%s> ChangeTeam() CTMDBG , team %d , req team %d willSwitch` format
- ClientPutInServer removed entity [] that was in player controller entindex