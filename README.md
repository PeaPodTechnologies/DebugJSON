# DebugJSON
 An Arduino library for sending debug messages and recieving commands in JSON format over a serial interface.

## Paradigm

The library is designed to be used in a system where a microcontroller is connected to a host computer via a serial interface. The microcontroller can send telemetry data and debug messages to be read by the host computer, and the host computer can send commands and configuration to be interpreted by the microcontroller. The messages and commands are sent in JSON format (UTF-8, `\n`-terminated) to allow for the simplest possible cross-platform lightweight de-/serialization of structured data.

## Usage

Think of each message as a self-instruction from a machine with two halves, the microcontroller and the host computer. Rather than define two different schemas, the library uses a single schema for all messages, and the two halves of the machine can interpret the messages as they see fit. This allows for self-spoofing; consider the examples in the case of a "headless" microcontroller self-instructing based on pre-programmed rules (for hardware debugging), or a simulation of the microcontroller running on the host computer (for UX scalability planning).

## Schema

Based on MQTT and ROS, the schema is designed to be as simple as possible while still being able to represent a wide variety of messages. The schema is a JSON object with a `msg` field, and a `type` field that is used to determine the message's purpose.

```typescript
{
  "msg": string, // Or '\0' for heartbeat
} & 
({
  "type": "revision", // Indicates that the microcontroller is waiting to recieve instructions from the host computer
  "data": number, // The version of the schema that the microcontroller is using
} | {
  "type": "debug", // Direct exposure of the microcontroller's state (peripherals and internal)
  "data": string, // I.e. __FILE__ __LINE__ __FUNCTION__ "message"
} | {
  "type": "telemetry",  // User-friendly representation of the microcontroller's peripheral state (sensor)
  "data": {
    [key: string]: number,  // Lookup by Class ID; value units are class-specific
  },
} | {
  "type": "command", // Direct instruction to the microcontroller's peripheral state (actuator)
  "data": {
    [key: string]: number, // Lookup by Class ID; boolean (0 | !0) for on/off, number for PWM
  },
} | {
  "type": "config", // User-friendly instruction to the microcontroller's internal state (configuration)
  "data": {
    [key: string]: any, // Lookup & type-inference done by microcontroller software
  },
} | {
  "type": "heartbeat", // No data, just a message to indicate that the microcontroller is still alive
})
```