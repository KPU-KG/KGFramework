myid = -1

DIRECTION_N = 0
DIRECTION_S = 1
DIRECTION_W = 2
DIRECTION_E = 3

runaway_direction = 0
runaway_count = 0
runaway_player_id = -1

EVENT_ID_WAKEUP = 0
EVENT_ID_RANDOM_MOVE = 1
EVENT_ID_RUNAWAY = 2

function set_object_id( id )
	myid = id
end

function proc_wake_up( )
	lua_api_add_event(myid, EVENT_ID_RANDOM_MOVE, 1000)
end

function proc_random_move( )
	if(runaway_count == 0) then
		lua_api_move(myid, lua_api_get_random_direction())
		lua_api_add_event(myid, EVENT_ID_RANDOM_MOVE, 1000)
	end
end

function proc_runaway( )
	runaway_count = runaway_count - 1
	lua_api_move(myid, runaway_direction)
	if(runaway_count ~= 0) then
		lua_api_add_event(myid, EVENT_ID_RUNAWAY, 1000)
	else
		lua_api_send_chat(runaway_player_id, myid, "BYE")
		lua_api_add_event(myid, EVENT_ID_RANDOM_MOVE, 1000)
	end
end

function player_is_near( player )
	p_x = lua_api_get_x(player)
	p_y = lua_api_get_y(player)
	m_x = lua_api_get_x(myid)
	m_y = lua_api_get_y(myid)
	if(p_x == m_x) then
		if(p_y == m_y) then
			lua_api_send_chat(player, myid, "HELLO")
			prev_count = runaway_count
			runaway_direction = lua_api_get_random_direction()
			runaway_count = 3
			runaway_player_id = player
			if prev_count == 0 then
				lua_api_add_event(myid, EVENT_ID_RUNAWAY, 200)
			end
		end
	end
end


function call_npc_proc( event_id )
	if(event_id == EVENT_ID_WAKEUP) then
		proc_wake_up()
	end
	if(event_id == EVENT_ID_RANDOM_MOVE) then
		proc_random_move()
	end
	if(event_id == EVENT_ID_RUNAWAY) then
		proc_runaway()
	end
end