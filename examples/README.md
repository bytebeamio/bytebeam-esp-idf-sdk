# Bytebeam ESP SDK Examples
1. **basic_example**:
2. **esp32_touch**: Once configured, ESP32 will continuously measure capacitance of touch pad sensors. Measurement is reflected as numeric value inversely related to sensor's capacitance. With a finger touched on a pad, its capacitance will get larger meanwhile the measured value gets smaller, and vice versa. Next this example will make a payload JSON and publish touch values to Bytebeam cloud.
3. **sht31_temp_humid**: In this example we had configured ESP32 with SHT31 temperature and humidity sensor. Here we continuously measures temperature and humidity samples and send it to Bytenbeam cloud   