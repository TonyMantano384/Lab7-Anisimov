## Системный монитор
**Автор:** Анисимов Д.С., группа И-43 
## CPU Usage (Использование процессора):
Считывает загрузку CPU через GetSystemTimes.
Показывает количество ядер (GetSystemInfo) и частоту процессора (через реестр).
Обновляет данные каждую секунду.
![Картинка](docs/task1cpu.png)

## RAM Monitoring (Мониторинг памяти):
Использует GlobalMemoryStatusEx.
Отображает общий объем ОЗУ, использованную и свободную память (в ГБ), а также процент загрузки.
![Картинка](docs/task2ram.png)

## Network Traffic (Сетевой трафик):
Использует GetIfTable для получения статистики по сетевым интерфейсам.
Считает разницу в байтах между вызовами, чтобы показать текущую скорость приема (RX) и передачи (TX) данных.
![Картинка](docs/task3network.png)

## Disk I/O (Дисковый ввод-вывод)
Использует DeviceIoControl с параметром IOCTL_DISK_PERFORMANCE для получения статистики чтения/записи с диска C:.
Показывает скорость чтения и записи в МБ/с.
![Картинка](docs/task4disk.png)

## Battery Status (Статус батареи):
Использует GetSystemPowerStatus.
Показывает процент заряда, статус (заряжается/разряжается), подключена ли зарядка и оставшееся время работы.
![Картинка](docs/task5bat.png)

## Filesystem and Swap (Файловая система и подкачка):
Показывает свободное и занятое место на диске C: (через GetDiskFreeSpaceExA).
Показывает статистику использования файла подкачки (Swap/Pagefile) через GlobalMemoryStatusEx.
![Картинка](docs/task6file.png)

## Process Dynamics (Динамика процессов):
Делает снимок процессов (CreateToolhelp32Snapshot).
Считает общее количество процессов.
Ищет процесс, который потребляет больше всего оперативной памяти (через GetProcessMemoryInfo), и выводит его PID и имя.
![Картинка](docs/task7pro.png)

## System Load and Uptime (Нагрузка и время работы):
Показывает время с момента запуска системы (GetTickCount).
Дублирует текущую загрузку CPU.
![Картинка](docs/task8uptime.png)

## Network Connections (Сетевые подключения):
Использует GetTcpTable.
Считает количество TCP-соединений и группирует их по состояниям (ESTABLISHED, LISTEN, TIME_WAIT и т.д.).
![Картинка](docs/task9connect.png)

## Thermal Status (Тепловой статус):
Примечание: В данном коде этот пункт не считывает температуру напрямую (так как это сложно сделать стандартными средствами без драйверов), но он показывает имя процессора (из реестра) и статус питания/батареи.
![Картинка](docs/task10Temp.png)

## Как запустить 
1. Открыть файл docs/ConsoleApplication21.sln в SharpDevelop или Visual Studio 
2. Нажать F5 
3. Появятся окна
## Скриншоты 
### Скриншот 1: Меню 
 ![Картинка](docs/MENU.png) 
### Скриншот 2: опция CPU
 ![Картинка](docs/CPU.png) 
### Скриншот 3: опция RAM
 ![Картинка](docs/RAM.png) 
### Скриншот 4: опция NETWORK TRAFFIC
 ![Картинка](docs/NETWORK.png) 
### Скриншот 5: опция I/O
 ![Картинка](docs/IO.png) 
### Скриншот 6: опция BATTERY STATUS
 ![Картинка](docs/BAT.png) 
### Скриншот 7: опция FILESYSTEM and SWAP
 ![Картинка](docs/file.png) 
### Скриншот 8: опция Process Dynamics
 ![Картинка](docs/process.png)
### Скриншот 9: опция SYSTEEM LOAD AND UPTIME
 ![Картинка](docs/uptime.png)  
### Скриншот 10: опция NETWORK CONNECTIONS
 ![Картинка](docs/connect.png) 
### Скриншот 11: опция THERMAL STATUS
 ![Картинка](docs/thermal.png)
