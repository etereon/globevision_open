script_name("globevision map")
script_author("etereon")

local imgui = require("mimgui")
local ffi = require("ffi")

-- определние функций WinAPI
ffi.cdef("unsigned long __stdcall GetModuleHandleA(const char*);")
ffi.cdef("unsigned long __stdcall GetProcAddress(unsigned long, const char*);")

-- определение структур библиотеки
ffi.cdef [[
  typedef unsigned char  uint8_t;
  typedef unsigned short uint16_t;
  typedef unsigned int   uint32_t;

  typedef struct position {
    float x, y, z;
  } position_t;

  typedef struct quat {
    float w, x, y, z;
  } quat_t;

  typedef struct player_data {
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

local plugin = ffi.C.GetModuleHandleA("globevision.asi") -- получение хендла библиотеки

local gv = { -- получение экспортируемых функций
  create_buffer = ffi.cast("int(__stdcall*)(const char*)", ffi.C.GetProcAddress(plugin, "create_buffer")),
  update_buffer = ffi.cast("void(__stdcall*)(int)", ffi.C.GetProcAddress(plugin, "update_buffer")),
  get_player_data = ffi.cast("player_data_t(__stdcall*)(int, uint16_t)", ffi.C.GetProcAddress(plugin, "get_player_data")),
}

--------------------------------------------------------------------------------------------------------------------------

local ImVec2 = imgui.ImVec2
local ImVec4 = imgui.ImVec4

local servers = {
  ips = {
    "54.37.142.72",
    "54.37.142.73",
    "54.37.142.74",
    "54.37.142.75",
    "51.83.153.240",
    "185.169.134.3",
    "185.169.134.4",
    "185.169.134.43",
    "185.169.134.44",
    "185.169.134.45",
    "185.169.134.5",
    "185.169.134.59",
    "185.169.134.61",
    "185.169.134.107",
    "185.169.134.109",
    "185.169.134.166",
    "185.169.134.171",
    "185.169.134.172",
    "185.169.134.173",
    "185.169.134.174",
    "80.66.82.191",
    "80.66.82.190",
    "80.66.82.188",
    "80.66.82.168",
    "80.66.82.159",
    "80.66.82.200",
    "80.66.82.144",
    "80.66.82.132",
    "80.66.82.128",
    "80.66.82.113",
    "80.66.82.82",
    "80.66.82.87"
  },
  names = {
    "Advance RP Red",
    "Advance RP Green",
    "Advance RP Blue",
    "Advance RP Lime",
    "Advance RP Chocolate",
    "Arizona RP Phoenix",
    "Arizona RP Tucson",
    "Arizona RP Scottdale",
    "Arizona RP Chandler",
    "Arizona RP Brainburg",
    "Arizona RP Saint Rose",
    "Arizona RP Mesa",
    "Arizona RP Red Rock",
    "Arizona RP Yuma",
    "Arizona RP Surprise",
    "Arizona RP Prescott",
    "Arizona RP Glendale",
    "Arizona RP Kingman",
    "Arizona RP Winslow",
    "Arizona RP Payson",
    "Arizona RP Gilbert",
    "Arizona RP Show-Low",
    "Arizona RP Casa Grande",
    "Arizona RP Page",
    "Arizona RP Sun City",
    "Arizona RP Queen Creek",
    "Arizona RP Sedona",
    "Arizona RP Holiday",
    "Arizona RP Wednesday",
    "Arizona RP Yava",
    "Arizona RP Faraway",
    "Arizona RP Bumble Bee"
  }
}

local menu = {
  names = imgui.new['const char*'][#servers.names](servers.names),
  address = imgui.new.char[16](""),
  nickname = imgui.new.char[20](""),
  state = imgui.new.bool(true),
  scale = imgui.new.float(1),
  server = imgui.new.int(0),
  width = 215,
  image = 0
}

local map = {
  size = ImVec2(500.0, 500.0),
  zoom = imgui.new.float(1),
  buffer_id = -1,
  focused_id = -1
}

function split_rgba(rgba)
  local r = bit.band(bit.rshift(rgba, 24), 0xFF)
  local g = bit.band(bit.rshift(rgba, 16), 0xFF)
  local b = bit.band(bit.rshift(rgba, 8), 0xFF)
  local a = bit.band(rgba, 0xFF)
  return r, g, b, a
end

function get_player_color4(rgba)
  local r, g, b = split_rgba(rgba)
  return ImVec4(r / 255, g / 255, b / 255, 1)
end

function main()
  if plugin == 0 then -- если библиотека не найдена, то завершаем скрипт
    print("Error getting the library descriptor. Please install globevision.asi")
    return thisScript():unload()
  end

  sampRegisterChatCommand("gvmap", function() menu.state[0] = not menu.state[0] end)
  imgui.OnFrame(function() return menu.state[0] end, imgui_draw_handler)
  imgui.OnInitialize(imgui_init_handler)
  
  while true do
    if map.buffer_id ~= -1 and menu.state[0] then
      gv.update_buffer(map.buffer_id)
    end
    wait(100)
  end
end

function imgui_init_handler()
  imgui.SwitchContext()
  local style = imgui.GetStyle()

  style.WindowRounding         = 1.2
  style.WindowBorderSize       = 0

  menu.image = imgui.CreateTextureFromFile(getWorkingDirectory() .. "/resource/globevision/map.jpg")
end

function imgui_draw_handler(player)
  local size = map.size * menu.scale[0]

  imgui.PushStyleVarVec2(imgui.StyleVar.WindowPadding, ImVec2(0, 0))
  imgui.SetNextWindowSize(ImVec2(menu.width, 20) + size, imgui.Cond.Always)
  imgui.Begin("GlobeVision Map", menu.state, imgui.WindowFlags.NoResize)
  imgui.PopStyleVar()

  imgui.BeginChild("##menu", ImVec2(menu.width, imgui.GetContentRegionAvail().y), true)

  imgui.PushItemWidth(imgui.GetContentRegionAvail().x)
  if imgui.Combo("##servers_list", menu.server, menu.names, #servers.names) then
    imgui.StrCopy(menu.address, servers.ips[menu.server[0] + 1])
  end
  imgui.PopItemWidth()

  imgui.InputTextWithHint("##ip", "Server IP", menu.address, 16)
  imgui.SameLine()

  if imgui.Button("Apply", ImVec2(imgui.GetContentRegionAvail().x, 20)) then
    map.buffer_id = gv.create_buffer(menu.address)
  end

  imgui.Text(string.format("Current buffer: %d", map.buffer_id))

  imgui.Separator()

  imgui.SliderFloat("Scale", menu.scale, 1.0, 2.5)
  imgui.SliderFloat("Zoom", map.zoom, 1.0, 100.0)

  imgui.PushItemWidth(imgui.GetContentRegionAvail().x)
  imgui.InputTextWithHint("##search", "Name to find", menu.nickname, 20)

  if imgui.Button("Reset focus", ImVec2(imgui.GetContentRegionAvail().x, 20)) then
    map.focused_id = -1
  end

  local focused = {
    exists = false,
    data = 0
  }

  if map.buffer_id ~= -1 and map.focuses_id ~= -1 then
    focused.data = gv.get_player_data(map.buffer_id, map.focused_id)
    focused.exists = focused.data.updated
  end

  draw_list(focused)

  imgui.EndChild()

  imgui.PushStyleVarVec2(imgui.StyleVar.ItemSpacing, ImVec2(0, 0))
  imgui.SameLine(menu.width - 1)
  imgui.PopStyleVar()

  draw_map(size, focused)

  imgui.End()
end

function draw_list(focused)
  imgui.PushStyleVarVec2(imgui.StyleVar.WindowPadding, ImVec2(4, 4))
  imgui.BeginChild("##list", imgui.GetContentRegionAvail() - ImVec2(0, focused.exists and 126 or 28), true)

  local name_to_find = ffi.string(menu.nickname)
  local count = 0

  if map.buffer_id ~= -1 then
    for i = 0, 1004 do
      local player = gv.get_player_data(map.buffer_id, i)
      if player.updated then
        local name = ffi.string(player.nickname)
        if name_to_find:len() == 0 or name:lower():find(name_to_find) ~= nil then
          imgui.PushStyleColor(imgui.Col.Text, get_player_color4(player.color))
          if imgui.Selectable(string.format("%s (%d)", name, i), i == map.focused_id) then
            map.focused_id = i
          end
          imgui.PopStyleColor()
        end
        count = count + 1
      end
    end
  end

  imgui.EndChild()

  imgui.BeginChild("##count", ImVec2(imgui.GetContentRegionAvail().x, 24), true)
  imgui.PopStyleVar()
  imgui.Text(string.format("Listed: %d", count))
  imgui.EndChild()

  if focused.exists then
    local data = focused.data
    imgui.BeginChild("##data", imgui.GetContentRegionAvail(), true)
    imgui.Text("Selected player data")
    imgui.Separator()
    imgui.Text(string.format("Health: %d\nArmor: %d\nModel: %d\nWeapon: %d", data.health, data.armor, data.model, data.weapon))
    imgui.EndChild()
  end
end

function draw_map(size, focused)
  imgui.PushStyleVarVec2(imgui.StyleVar.WindowPadding, ImVec2(0, 0))
  imgui.BeginChild("##map", imgui.GetContentRegionAvail(), true)
  imgui.PopStyleVar()

  local zoom = ImVec2(225 * (map.zoom[0] - 1), 225 * (map.zoom[0] - 1))

  local dl = imgui.GetWindowDrawList()
  local pos = imgui.GetWindowPos()

  size = size + zoom

  if focused.exists then
    pos.x = pos.x - focused.data.position.x / (6000 / size.x) - zoom.x / 2
    pos.y = pos.y + focused.data.position.y / (6000 / size.y) - zoom.y / 2
  else
    pos = pos - zoom * 0.5
  end

  dl:AddImage(menu.image, pos, pos + size)

  if map.buffer_id == -1 then return imgui.EndChild() end

  for i = 0, 1004 do
    local player = gv.get_player_data(map.buffer_id, i)
    if player.updated then
      local x = (size.x / 2) + (player.position.x / (6000 / size.x))
      local y = (size.y / 2) - (player.position.y / (6000 / size.y))

      dl:AddCircleFilled(pos + ImVec2(x + 1, y + 1), 5, 0xFF000000, 24)
      dl:AddCircleFilled(pos + ImVec2(x, y), 5, imgui.GetColorU32Vec4(get_player_color4(player.color)), 24)
    end
  end

  imgui.EndChild()
end
