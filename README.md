# globevision_open
> Открытая версия библиотеки для обмена информацией об игроках для SA:MP 0.3.7 (R1, R3-1), а также [RakSAMP Lite](https://www.blast.hk/threads/108052/)

Позволяет каждому пользователю автоматически отправлять информацию о зоне стрима в единый пул, запрашивать данные о других игроках. От подобных себе проектов (GMap и т.д) отличается возможностью масштабного использования и наличием экспортов, которые позволяют использовать информацию в любом виде (не только карты и т.д). Основанные на данной библиотеке работы:
* [Web-карта всех поддерживаемых серверов](https://neverlane.one/gvision/) (by @neverlane, архивный скриншот)
![1703447469974](https://github.com/etereon/globevision_open/assets/67074433/625d4988-5e84-4ef6-ac78-ee1084b8a536)
* [Локальная карта с возможностью слежки и эмуляции синхронизации (Lua)](https://www.blast.hk/threads/196972/)
## Возможности
* Автоматический сбор информации об игроках посредством перехвата входящящей синхронизации (онфут, водительской и пассажирской) и последующая её отправка на центральный сервер​
* Получение данных об игроках, которые находятся вне вашей стрим-зоны, либо на другом сервере, но попадают зону видимости другого пользователя плагина​
## Использование
Сам плагин не содержит никакого обработчика входящей информации, но экспортирует функции, которые дают возможность получать данные с сервера и использовать их в любом виде, например, в рамках своего приложения или Lua-скрипта (экспортируемые функции могут быть вызваны через FFI), т.е. использование ограничено лишь вашей фантазией
## Как это работает?
Работоспособность достигается за счёт клиент-сайд плагина, собирающего информацию об игроках, и сервера, который хранит полученную информацию, либо отдаёт её по запросу. Для организации адекватного хранения данных я поступил следующим образом:​
1. Файл конфигурации сервера содержит в себе IP-адреса поддерживаемых игровых серверов​
2. При старте сервер выделяет под каждый IP-адрес свой буфер, который содержит в себе структуры MAX_PLAYERS = 1005 игроков следующего вида:
```cpp
struct player_data_t {
  uint8_t updated : 1; // если == 1, значит игрок был обновлён в течение последних 5-и секунд
  uint8_t health : 7;
  uint8_t armor;
  uint16_t weapon : 6; // текущее оружие
  uint16_t model : 10; // скин
  position_t position; // позиция
  quat_compressed_t quaternion; // поворот
  uint32_t color;
  char nickname[20];
 };
 
 // в будущем может быть добавлено больше данных
```
3. Плагин автоматически получает с сервера список поддерживаемых адресов при запуске​
4. Текущая версия даёт возможность добавить в список 255 адресов​
5. Данные с других игровых серверов не собираются​

Библиотека является открытой, любой желающий может запустить её на своём хосте и использовать отдельно. Помимо этого, я поддерживаю работу сервера для массового использования, к которому привязан плагин, прикреплённый в этой теме. С актуальным списком адресов, добавленных мною, можно ознакомиться тут (буду постепенно пополнять его, в т.ч по просьбам)
## Доступные функции и структуры
Для обеспечения неблокируемой работы скриптов я создал систему буферов, которые по очереди заполняются в потоке плагина. Функции для работы с ними:
```cpp
struct position_t {
  float x, y, z;
};

struct quat_t {
  float w, x, y, z;
};

struct player_data_local_t {
  bool updated;
  uint8_t health;
  uint8_t armor;
  uint16_t weapon;
  uint16_t model;
  position_t position;
  quat_t quaternion;
  uint32_t color;
  char nickname[21];
};

// создание буфера по IP-адресу сервера, возвращает номер созданного буфера
// Если буфер уже существует, то вернёт номер существующего буфера
int __stdcall create_buffer(const char* ip);

// добавление буфера в очередь на обновление
void __stdcall update_buffer(int buffer_id);

// долучение данных об игроке из буфера
player_data_local_t __stdcall get_player_data(int buffer_id, uint16_t player_id);
```
## Примеры кода (Lua)
Примеры использования и Lua-библиотека доступны в директориях lua_library/lua_examples
