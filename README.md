# caen743

Globus-M2 Core Thomson Scattering diagnostics control.

## Сборка
сейчас для сборки используется cygwin

## Запуск и использование
Пока предлагается запускать исполняемый файл из папки со сборкой: D:\code\caen743\cmake-build-debug\fastAcquisition.exe.
Также возможен запуск из среды разработки CLion.

1. Подать питание на платы CAEN (включить корзину).  
Либо удерживая тумблер On-Off на лицевой панели в верхнем положении, 
либо через программу C:\Program Files (x86)\CAEN\CAENCrateToolBox\CAEN_V8100\Demo\V8100Demo\VME8100_Manager.jar, 
указав следующий IPv4 адрес: 192.168.10.43.  
На нижней панели корзины загорится зелёный индикатор рядом с тумблером. 
На всех платах оцифровщиков загорятся некоторые индикаторы, которые через несколько секунд сменятся на рабочие: 
NIM, зелёный link, pll Lock на ведущей плате и clk in на ведомых.  
Link не будет гореть, пока компьютер системы сбора выключен или не подключено оптоволокно. 
Если состояние индикаторов не приходит к рабочему через несколько секунд после подачи питания, 
возможно, это проблема 1.  
Оставить оцифровщики выходить на температурный режим в течение получаса.
2. Включить компьютер системы сбора.  
Под крышкой на лицевой панели необходимо нажать и отпустить подпружиненный чёрный тумблер.
Логин: ts_group
3. Проверить файл конфигурации системы сбора D:\data\fastDump\config.json  
Если файл отсутствует или некорректен будут использованы настройки по умолчанию.  
Чтение файла производится только при запуске программы.
Если необходимо изменить настройки, программу надо остановить, внести изменения в D:\data\fastDump\config.json
и запустить программу вновь (следующий пункт).
4. Запустить серверную часть ПО: D:\code\caen743\cmake-build-debug\fastAcquisition.exe  
Первой строкой в открывшейся консоли будет версия программы.
Далее следует сообщение о чтении файла конфигурации.
После чего начнётся последовательное подключение к каждой из запрошенных в конфигурации плат.  
Во время установки соединения на текущей плате горит жёлтый link.
При успешном опросе всех плат появляется сообщение «System initialised», иначе – программа завершает работу.
5. Подключиться к серверу с помощью любого клиента.  
На текущий момент создан Python класс [Chatter](/utils/chatter.py), реализующий протокол обмена сообщениями [API](#api). 
Пример использования приведён в файле [control.py](utils/control.py). 
Данный алгоритм устанавливает соединение, 
посылает команду на ожидание триггера и спустя заданный промежуток времени останавливает сбор.  
Также реализован алгоритм для просмотра и стат-обработки полученных данных [viewer.py](utils/viewer.py).

Штатный режим работы предполагает единовременное выполнение пунктов 1-4. 
После чего можно многократно выполнять подключение клиента или многократно посылать запросы из одного и того же клиента.


## Файл конфигурации
Файл структурирован по подсистемам:
- crate
    - caenCount = целочисленное число используемых плат от [1, 5).  
Используются первые caenCount оптоволокон для связи. 
По умолчанию величина = 4.  
Пример применения: для отладки удобно работать с одной платой. 
Значение устанавливается 1, нужная плата подключается к 0му гнезду контроллера на задней панели компьютера. 
Для удобства номера гнёзд соответствуют номерам оптоволокон. 
Нулевой кабель не маркирован (надо исправить!).
- experiment
    - maxAcquisitionTime = положительное число с плавающей точкой, секунды.  
Максимальное время нахождения платы в состоянии ожидания триггера.
Не реализовано в текущей версии!
    - debugCounter = целочисленное, неотрицательное.  
Счётчик отладочных разрядов. 
Не реализовано в текущей версии!
    - plasmaCounter = целочисленное, неотрицательное.  
Счётчик разрядов с плазмой. 
Не реализовано в текущей версии!
    - globusCounter = целочисленное, неотрицательное.  
Последний номер разряда токамака, в котором проводились измерения ТР. 
Не реализовано в текущей версии!
    - isPlasma = {true, false}.  
Следующее измерение с плазмой?
Не реализовано в текущей версии!
- caen
    - recordLength = натуральное число из диапазона [1, 65)*16.  
Число временнЫх точек в одном событии. 
Число не должно превышать 1024 и должно быть кратно 16.
По умолчанию 1024.
    - frequency = {“3.2”, “1.6”, “0.8”, “0.4”}.  
Частота оцифровки в GS/s. 
Величина записывается в виде строковой переменной.
Время между соседними точками оцифровки вычисляется как 1/*frequency* и соответственно равно 
0.3125, 0.625, 1.25 и 2.5 нс.
По умолчанию “3.2”.
    - triggerDelay = натуральное число не более 255.  
Перезапись кольцевого буфера будет остановлена после прихода триггера спустя triggerDelay*16/ frequency. 
Увеличение данного параметра приводит к смещению осциллограммы влево в окне оцифровки.
По умолчанию 18.
    - offset = число с плавающей точкой из области \[-1250, 1250\].  
Задаёт смещение окна оцифровки по вертикали в милливольтах.  
При нулевом значении вертикальная развёртка составляет от -1250 до 1250 мВ.
Задав смещение 1100 мВ, окно оцифровки становится от -150 до 2350 мВ.  
Внутри программы число переводится в число в шестнадцатеричной системе.
Значения связаны формулой:  
Offset_mV * 0xFFFF = 2500 * (0x7FFF - Offset_Hex)    
![documentation on DC offset](/res/offset.png "Image from CAEN datasheet.")  
Регистры 16-бит ЦАПа, которым задаётся опорный уровень ОУ (почему два выхода???).
Документация говорит, что  
0000 => окно оцифровки от -2.5 до 0,  
7FFF => окно оцифровки от -1.25 до 1.25,  
FFFF => окно оцифровки от 0 до 2.5.  
По умолчанию 1100.0.
    - triggerThreshold = число с плавающей точкой из области \[-1250 + *offset*, 1250 + *offset*\].  
Задаёт порог запуска по 0-му каналу в милливольтах.  
Порог не привязан к окну оцифровки SAMLONG: ноль в чипе "плавающий" и имеет тенденцию сползать,
в то время как компаратор работает более честно по моим наблюдениям.  
Внутри программы число переводится в число в шестнадцатеричной системе.
Значения связаны формулой:  
Trig_mV = 1250 + *offset_mV* - Trig_Hex * 2500 / 0xFFFF  
Регистры 16-бит ЦАПа, подключенного к компаратору, 
который сравнивает его с уровнем сигнала после подстройки постоянного смещения *offset*. 
Документация говорит, что  
0000 = +1.25 V,  
7FFF = 0 V,  
FFFF = -1.25 V.  
По умолчанию 300.0.
- connection
    - connectionInterval =  неотрицатильное число.  
Время в секундах между попытками установить соединение с клиентом.
Увеличение данного параметра уменьшает скорость установки соединения, 
зато снижает нагрузку на ЦП во время ожидания клиента.
По умолчанию 1.
    - commandTimeout = неотрицательное число.  
Время в миллисекундах между чтениями входного буфера команд от клиента.
Увеличение данного параметра уменьшает скорость реакции, зато снижает нагрузку на ЦП.
По умолчанию 100.
    - connectionDeadTime = неотрицательное число.  
Время жизни соединения в секундах после последнего успешного запроса от клиента. 
Пока клиент не разорвал соединение или не признан «неживым», подключение нового невозможно. 
Если клиент не посылал запросы более чем  connectionDeadTime секунд, соединение принудительно закрывается.
По умолчанию 120.
    - messagePoolingInterval = неотрицательное число.  
Время в миллисекундах между чтениями внутренней очереди сообщений. 
Увеличение данного параметра уменьшает скорость реакции, зато снижает нагрузку на ЦП.
По умолчанию 100.
- storage
    - plasmaPath = “кошерный\виндовский\путь”  
строка с путём (абсолютный работает, относительный – не пробовал) к существующей папке для хранения разрядов с плазмой.
По умолчанию "d:/data/fastDump/plasma/".
    - debugPath = “кошерный\виндовский\путь”  
строка с путём (абсолютный работает, относительный – не пробовал) к существующей папке для хранения отладочных разрядов.
По умолчанию "d:/data/fastDump/debug/".
    - logPath = “кошерный\виндовский\путь”  
строка с путём (абсолютный работает, относительный – не пробовал) к существующей папке для хранения журналов.
По умолчанию "d:/data/fastDump/logs/".



