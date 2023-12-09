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
  } player_data_t; // на самом деле это player_data_local_t
]]

local plugin = ffi.C.GetModuleHandleA("globevision.asi") -- получение хендла библиотеки

local gv = { -- получение экспортируемых функций
  create_buffer = ffi.cast("int(__stdcall*)(const char*)", ffi.C.GetProcAddress(plugin, "create_buffer")),
  update_buffer = ffi.cast("void(__stdcall*)(int)", ffi.C.GetProcAddress(plugin, "update_buffer")),
  get_player_data = ffi.cast("player_data_t(__stdcall*)(int, uint16_t)", ffi.C.GetProcAddress(plugin, "get_player_data")),
}

function main()
  if plugin == 0 then -- если библиотека не найдена, то завершаем скрипт
    print("Error getting the library descriptor. Please install globevision.asi")
    return thisScript():unload()
  end

  local buffer = gv.create_buffer("80.66.82.82") -- создаём буфер для Arizona RP Faraway
  gv.update_buffer(buffer) -- добавляем буфер в очередь на обновление

  wait(100) -- ждём заполнение буфера (в зависимости от интернета происходит за 10-100мс)

  local player = gv.get_player_data(buffer, 71) -- получаем данные игрока с ID = 71

  if player.updated then -- если данные по игроку есть, то выведем его координаты
    print(string.format("%f %f %f", player.position.x, player.position.y, player.position.z))
  end
end
