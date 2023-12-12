local gv = require("globevision")

function main()
  if not gv.initialize() then -- завершаем скрипт, если библиотека не инициализировалась
    return thisScript():unload()
  end

  local buffer = gv.create_buffer("80.66.82.168") -- создаём буфер для Arizona RP Page
  gv.update_buffer(buffer) -- добавляем буфер в очередь на обновление

  wait(100) -- ждём заполнение буфера (в зависимости от интернета происходит за 10-100мс)

  local player = gv.get_player_data(buffer, 403) -- получаем данные игрока с ID = 403

  if player.updated then -- если данные по игроку есть, то выведем их
    print(string.format("NAME: %s | POS: %f %f %f", player.nickname, player.position.x, player.position.y, player.position.z))
  end
end
