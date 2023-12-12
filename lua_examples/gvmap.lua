script_name("globevision map")
script_author("etereon")

local vector3d = require("vector3d")
local gv = require("globevision")
local imgui = require("mimgui")
local ffi = require("ffi")

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
    "80.66.82.87",
    "185.169.134.67",
    "185.169.134.91",
    "91.215.86.30",
    "91.215.86.6",
    "91.215.86.8"
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
    "Arizona RP Bumble Bee",
    "Evolve RP Saint-Louis",
    "Evolve RP Cleveland",
    "Samp-Rp.Ru Underground",
    "Samp-Rp.Ru Revolution",
    "Samp-Rp.Ru Legacy"
  }
}

local menu = {
  names = imgui.new['const char*'][#servers.names](servers.names),
  address = imgui.new.char[16](""),
  nickname = imgui.new.char[20](""),
  state = imgui.new.bool(false),
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

local sync = {
  spectate = false,
  update = false,
  list = {},
  pos = 0
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

function remove_players()
  for i = 1, #sync.list do
    print(sync.list[i])
    gv.world_player_remove(sync.list[i])
  end
  sync.list = {}
end

function add_players(i, data)
  if map.buffer_id == -1 or sampGetCurrentServerAddress() ~= ffi.string(servers.ips[menu.server[0] + 1]) then
    return
  end

  for i = 0, gv.MAX_PLAYERS do
    local player = gv.get_player_data(map.buffer_id, i)
    local exists = sampGetCharHandleBySampPlayerId(i)
    if player.updated and not exists then
      gv.world_player_add(i, player)
      table.insert(sync.list, i)
    end
  end
end

function update_players()
  for i = 1, #sync.list do
    local player = gv.get_player_data(map.buffer_id, sync.list[i])
    if player.updated then
      gv.emulate_player_sync(sync.list[i], player)
    end
  end
end

function toggle_spectate(enabled)
  local bs = raknetNewBitStream()

  raknetBitStreamWriteInt32(bs, enabled)
  raknetEmulRpcReceiveBitStream(124, bs)

  raknetDeleteBitStream(bs)
end

function spectate_player(id)
  local bs = raknetNewBitStream()

  raknetBitStreamWriteInt16(bs, id)
  raknetBitStreamWriteInt8(bs, 0)
  raknetEmulRpcReceiveBitStream(126, bs)

  raknetDeleteBitStream(bs)
end

function main()
  if not gv.initialize() then -- если библиотека не инициализирована, то завершаем скрипт
    return thisScript():unload()
  end

  sampRegisterChatCommand("gvspec", gvspec_handler)
  sampRegisterChatCommand("gvmap", function() menu.state[0] = not menu.state[0] end)
  
  imgui.OnFrame(function() return menu.state[0] end, imgui_draw_handler)
  imgui.OnInitialize(imgui_init_handler)
  
  while true do
    if map.buffer_id ~= -1 and (sync.update or menu.state[0]) then
      gv.update_buffer(map.buffer_id)
      if sync.update and sampGetCurrentServerAddress() == ffi.string(servers.ips[menu.server[0] + 1]) then
        add_players()
        update_players()
      end
    end
    wait(sync.update and 100 or 300)
  end
end

function onSendPacket(id, bs, priority, reliability, orderingChannel) 
  return (sync.spectate and id == 212) == false
end

function onSendRpc(id, bs, priority, reliability, orderingChannel, shiftTs) 
  if sync.spectate and id == 52 then
    sync.spectate = false
    return false
  end
  return true
end

function gvspec_handler(params)
  local player_id = params:match("(%d+)")

  if player_id ~= nil then
    if not sync.spectate then
      sync.pos = vector3d(getCharCoordinates(PLAYER_PED))
    end
    sync.spectate = true
    toggle_spectate(1)
    spectate_player(player_id)
  elseif sync.pos ~= 0 then
    toggle_spectate(0)
    setCharCoordinates(PLAYER_PED, sync.pos:get())
    sync.pos = 0
  end
end

function imgui_init_handler()
  imgui.SwitchContext()
  local style = imgui.GetStyle()

  style.WindowRounding         = 1.2
  style.WindowBorderSize       = 0

  menu.image = imgui.CreateTextureFromFile(getWorkingDirectory() .. "/resource/gvmap/map.jpg")
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
  imgui.BeginChild("##list", imgui.GetContentRegionAvail() - ImVec2(0, focused.exists and 198 or 100), true)

  local name_to_find = ffi.string(menu.nickname)
  local count = 0

  if map.buffer_id ~= -1 then
    for i = 0, gv.MAX_PLAYERS do
      local player = gv.get_player_data(map.buffer_id, i)
      if player.updated then
        local name = player.nickname
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

  local width_x = imgui.GetContentRegionAvail().x

  imgui.BeginChild("##count", ImVec2(width_x, 24), true)
  imgui.PopStyleVar()
  imgui.Text(string.format("Listed: %d", count))
  imgui.EndChild()

  if focused.exists then
    local data = focused.data
    imgui.BeginChild("##data", ImVec2(width_x, 94), true)
    imgui.Text("Selected player data")
    imgui.Separator()
    imgui.Text(string.format("Health: %d\nArmor: %d\nModel: %d\nWeapon: %d", data.health, data.armor, data.model, data.weapon))
    imgui.EndChild()
  end

  if imgui.Button("World add players", ImVec2(width_x, 0)) then add_players() end
  if imgui.Button("World remove players", ImVec2(width_x, 0)) then remove_players() end

  if imgui.Button(sync.update and "Stop sync update" or "Start sync update", ImVec2(width_x, 0)) then
    sync.update = not sync.update
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

  for i = 0, gv.MAX_PLAYERS do
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
