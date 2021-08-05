--[[
@title shoot on USB
@chdk_version 1.4.1
]]

-- Enable Remote: false  Enable script start: false
-- (no conflict with USB usage)
-- Press Shutter to start script

-- switch to shooting node
if ( get_mode() == false ) then
        print("switching to shooting mode")
        set_record(1)
        while ( get_mode() == false ) do sleep(200) end
end

repeat
	if ( is_pressed("remote") ) then
		shoot()
		while ( is_pressed("remote") ) do
			sleep(100)
		end
	end

	sleep(100)

until is_pressed("video")

