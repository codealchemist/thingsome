# thingsome
NodeMCU connected thing proof of concept.

### Connection flow

When not setup:
- start access point
- receive a setup request
- save config
- connect to wifi network

When already setup:
- load config
- connect to wifi network

### HTTP Methods

- **setup:** `http://[deviceIp]/setup?ssid=[your-ssid]&pass=[your-pass]`

Sets up connection to your wifi network.

- **reset:** `http://[deviceIp]/reset-config`

Resets saved config (connection data to your wifi network).

- **retry login:** `http://[deviceIp]/retry-wifi-login`

Retries login to your wifi network.

- **start AP:** `http://[deviceIp]/start-access-point`

Starts access point.

- **disconnect station:** `http://[deviceIp]/disconnect-station`

Disconnects from your wifi network.

- **info:** `http://[deviceIp]/device-info`
Returns device info in query string format.
