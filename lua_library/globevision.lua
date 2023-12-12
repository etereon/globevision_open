local bswrite = require("bitstream_io").bs_write
local sampfuncs = require("sampfuncs")
local vector3d = require("vector3d")
local ffi = require("ffi")

local imports = {}
local mod = {}

mod.VERSION = "1.0b"
mod.MAX_PLAYERS = 1004

-- определние функций WinAPI
ffi.cdef("unsigned long __stdcall GetModuleHandleA(const char*);")
ffi.cdef("unsigned long __stdcall GetProcAddress(unsigned long, const char*);")

-- определение структур библиотеки
ffi.cdef [[
  typedef unsigned char  uint8_t;
  typedef unsigned short uint16_t;
  typedef unsigned int   uint32_t;

  typedef struct position_t {
    float x, y, z;
  } position_t;

  typedef struct quat_t {
    float w, x, y, z;
  } quat_t;

  typedef struct player_data_t {
    bool updated;
    uint8_t health;
    uint8_t armor;
    uint16_t weapon;
    uint16_t model;
    position_t position;
    quat_t quaternion;
    uint32_t color;
    char nickname[21];
  } player_data_t;
]]

local function compress_health_and_armor(health, armor)
	local hp = health >= 100 and 0xF0 or bit.lshift(health / 7, 4)
	local ap = armor >= 100 and 0x0F or bit.band(armor / 7, 0x0F)
	return bit.bor(hp, ap)
end

function mod.initialize()
  local library = ffi.C.GetModuleHandleA("globevision.asi") 

	if library == 0 then
    print("[GlobeVision] Error getting the library descriptor. Please install globevision.asi")
    return false
  end

	imports = {
		create_buffer = ffi.cast("int(__stdcall*)(const char*)", ffi.C.GetProcAddress(library, "create_buffer")),
  	update_buffer = ffi.cast("void(__stdcall*)(int)", ffi.C.GetProcAddress(library, "update_buffer")),
  	get_player_data = ffi.cast("struct player_data_t(__stdcall*)(int, uint16_t)", ffi.C.GetProcAddress(library, "get_player_data")),
	}

	return true
end

function mod.create_buffer(ip)
	if imports.create_buffer == nil then
		print("[GlobeVision] Error getting the create_buffer function address. Please install globevision.asi")
		return -1
	end

	return imports.create_buffer(ip)
end

function mod.update_buffer(id)
	if imports.update_buffer == nil then
		print("[GlobeVision] Error getting the update_buffer function address. Please install globevision.asi")
		return -1
	end

	return imports.update_buffer(id)
end

function mod.get_player_data(buffer_id, player_id)
	if imports.get_player_data == nil then
		print("[GlobeVision] Error getting the get_player_data function address. Please install globevision.asi")
		return -1
	end

  local cdata = imports.get_player_data(buffer_id, player_id)

	return {
    updated = cdata.updated,
    health = cdata.health,
    armor = cdata.armor,
    weapon = cdata.weapon,
    model = cdata.model,
    position = vector3d(cdata.position.x, cdata.position.y, cdata.position.z),
    quaternion = cdata.quaternion,
    color = cdata.color,
    nickname = ffi.string(cdata.nickname)
  }
end

function mod.world_player_add(player_id, data)
  if data.updated == nil or not data.updated then 
    return false 
  end

  local bs = raknetNewBitStream()

  bswrite.uint16(bs, player_id)
  bswrite.uint8(bs, 0)
  bswrite.uint32(bs, data.model)
  bswrite.vector3d(bs, data.position)
  bswrite.float(bs, 0)
  bswrite.uint32(bs, data.color)
  bswrite.uint8(bs, 0)
  bswrite.uint16(bs, 0)

  raknetEmulRpcReceiveBitStream(sampfuncs.RPC_SCRWORLDPLAYERADD, bs)
  raknetDeleteBitStream(bs)

  return true
end

function mod.world_player_remove(player_id)
  local bs = raknetNewBitStream()
  bswrite.uint16(bs, player_id)
  raknetEmulRpcReceiveBitStream(sampfuncs.RPC_SCRWORLDPLAYERREMOVE, bs)
  raknetDeleteBitStream(bs)
end

function mod.emulate_player_sync(player_id, data)
  if data.updated == nil or not data.updated then 
    return false 
  end

  local bs = raknetNewBitStream()

  bswrite.uint16(bs, player_id)
  
  bswrite.bool(bs, false) -- has lr keys
  bswrite.bool(bs, false) -- has ud keys

  bswrite.uint16(bs, 0) -- keys data
  bswrite.vector3d(bs, data.position)

  bswrite.normQuat(bs, {data.quaternion.w, data.quaternion.x, data.quaternion.y, data.quaternion.z})

  bswrite.uint8(bs, compress_health_and_armor(data.health, data.armor))
  bswrite.uint8(bs, data.weapon)
  bswrite.uint8(bs, 0) -- special action

  bswrite.compressedVector(bs, vector3d(0, 0, 0))

  bswrite.bool(bs, false) -- has surf data
  bswrite.bool(bs, false) -- has anim data

  raknetEmulPacketReceiveBitStream(sampfuncs.PACKET_PLAYER_SYNC, bs)
  raknetDeleteBitStream(bs)

  return true
end

return mod
