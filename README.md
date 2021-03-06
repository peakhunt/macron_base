# macron_base

## current status : trying to reach milestone-4

### Milestone-4
Goals
1. ~~Runtime configuration sync~~

2. common widgets for visual HMI development
   * ~~label widget with alarm indication~~
   * ~~gauge widget with alarm indication~~
   * ~~button widget with alarm & progress indication~~
   * ~~mimic framework for data & system visualization~~
   * common dashboard framework (alarm summary and etc.)

3. Control app library for faster & easier control app development on top of base framework.

4. WAGO I/O module integration

5. Advantech Local I/O module integration

![Linear & Radial Gauges](captures/gauges.png "linear and radial gauges")
![Widget Demo1](captures/widget_demo1.png "widget demo1")
![Widget Demo2](captures/widget_demo2.png "widget demo2")
![Mimic Demo](captures/mimic_demo.png "mimic demo")

### Milestone-3 (Reached on May/25/2018)
1. signal logger & realtime/offline tracing with line chart
   * Realtime Signal Logging & Graphing
   * Server side channel data logging
   * Offline channel data query & graphing
   * Channel data logging at 100ms resolution
   * many days of channel data logging at 100ms resolution (more than a GB data)

2. alarm logger
   * alarm event logging
   * alarm event query on client side
   * many years of alarm event storage

![Signal Trace Realtime](captures/signal_trace_realtime.png "realtime signal trace")
![Signal Trace Channel Select](captures/signal_trace_channel_select.png "trace channel select dialog")
![Signal Trace Settings](captures/signal_trace_settings.png "signal trace settings")
![Alarm Log](captures/alarm_log.png "alarm log")
![Signal Log Query](captures/signal_log_query.png "signal log query")
![Signal Log Setting](captures/channel_trace_setting.png "signal log setting")

### Milestone-2 (Reached on May/9/2018)
1. HMI Development for base system functionalities
   * Channel List
   * Channel Detail
   * Channel Configuration Update
   * Alarm List
   * Alarm Detail
   * Alarm Configuration Update
   * Current Alarms
   * HMI configuration with JSON file

2. HMI integration with Control Server
   * JSON based API for channel status query
   * JSON based API for alarm status query
   * System Project Configuration Loader API 
   * Alarm ACK API
   * JSON based API for channel configuration update
   * JSON based API for alarm configuration update
   * JSON based API for lookup table update
   * Automatic Channel Polling Scheduler based on configuration
   * Automatic Alarm Polling Scheduler based on configuration

3. Control Server Base Idea & Functionality Validation/Improvement
   * channel status polling verified
   * alarm status polling verified
   * configuration update verified

![Loading](captures/loading.png "main configuration loading")
![Main DashBoard](captures/current.png "dash board placeholder for visual HMI")
![Channel List](captures/channel_list.png "channel list")
![Channel Detail](captures/channel_detail.png "channel detail")
![Channel Config Update1](captures/channel_config_update1.png "channel configuration update")
![Channel Config Update2](captures/channel_config_update2.png "channel configuration update")
![Alarm List](captures/alarm_list.png "alarm list")
![Alarm Detail](captures/alarm_detail.png "alarm detail")
![Alarm Config Update](captures/alarm_config_update.png "alarm configuration update")
![Current Alarms](captures/current_alarms.png "current active alarms")

### Milestone-1 (Reached on April/26/2018)

1. Generic channel bsaed control/monitoring server.
   * Channels should be loaded from channel configuration database and, should be up and running.
   * Lookup table support for analog channels.
   * Sensor fault detection support.

2. Generic alarm based alarm server.
   * An alarm is associated with a channel.
   * Digital alarm support for digital channels.
   * Analog alarm support for analog channels. Support low/high alarm.
   * Alarm state machine and alarm delay

3. Generic Modbus Master Driver
   * Modbus registers should be mapped to each channel and updated accordingly.
   * Configuraable modbus master request schedule.
   * Multiple concurrent modbus masters via simple configuration.
   * Modbus TCP/RTU support

4. Generic Modbus Slave Driver
   * Modbus registers should be mapped to each channel and alarms, and updated accordingly.
   * Multiple concurrent modbus slaves via simple configuration.
   * Modbus TCP/RTU support

5. CLI (Command Line Interface Support)
   * telnet based CLI support for debugging/configuration/status monitoring/control.
   * Multiple concurrent telnet connection support

6. JSON Web Service support
   * JSON REST API support for status query for channel/alarm/system status
   * JSON REST API support for updating channel/alarm configurations.

7. Runtime configuration update
   * atomic power-fail safe runtime update for core alarm/channel configuration data via CLI/HMI.

8. JSON configuration file
   * Channels are configured on a project basis via JSON system configuration file.
   * Alarms are configured on a project basis via JSON system configuration file.
   * Modbus Masters  are configured on a project basis via JSON system configuration file.
   * Modbus Slaves are configured on a project basis via JSON system configuration file.

9. System architecture
   * Multi-threading support to fully utilize modern multi-core CPU.
   * Event-loop based program structure.


## blah blah blah

library dependencies

core --> modbus
     --> utils 

modbus ---> utils --->  libev
