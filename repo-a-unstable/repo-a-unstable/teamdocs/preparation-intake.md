# Preparations for The Green Mile intake with Tom

## Things that need to happen during intake

- Introducing each other
- Product vision
- Previous prototype of students that worked on the project before

- How he wants or visions to be involved in meetings etc, standups
- Location of meetings
- Meetings:
1. 4× MVP scope meetings
2. 4× MVP presentations
3. 1× Final presentation and handover

- Working facilities (preferred working place)

- Ask for permission to record the intake

## Questions we want to have answered

1. Where do we need to meet for the MVP's and final presentation?
2. Do you have a working facility? If yes, where and are we allowed to work/try things out there?
3. Where do we need to store our information?
4. What coding language do u need us to use in terms of what is compatible?
5. What is the general vision of the product?

## Answers on questions

- Question 1:



- Question 2:



- Question 3:



- Question 4:


## Task Preperation Intake 02.03.2026

### Where are the biggest technical risks:
- Humidity (water should not build up in the box).
- Weather change (box needs to withstand snow, rain, heat, ...).
- corrosion (system gets damaged and will not work correctly)
- correct air flow (Without a correct interpretation, it is impossible)
- Visibility of the LED (light needs to be bright enough to see it).
- Security on both Software and Hardware (DDos, getting access to embedded system)
- Capacity of the Battery 

### Power Optimization Strategy:
- Only check with sensors every 15 to 30 min.
- During break phase, put ESP32 into deep sleep.
- Every hour, send the data to the backend.

### How to clearly separate backend with embedded system:
- Embedded system only gathers information:
- Backend is controlling the embedded system.
- If connection to the backend is lost, then the embedded system may automatically go into deep sleep and gather.


### How many months can we get out of the battery:
- With optimizing data gathering and deep sleep, maybe 3-9 months
- depends on how often we use the sensors
- only sending information, when there is a change


## Intake wrote down 03.03.2026

### Key Objectives & Focus Areas

#### 1. **Extend Battery Life**
- **First Sprint:** Analyze battery behavior and identify energy consumption sources.
- Reverse engineering of prototypes to detect improvement opportunities and necessary fixes.
- Evaluate whether LEDs can be removed to reduce power consumption without compromising usability.

#### 2. **Hardware Optimization & Stability**
- Hardwire circuit boards to improve reliability and reduce power loss.
- Fix connection issues between the device and the dashboard (especially for the Wi-Fi version).
- Use **SIM cards** for stable and reliable internet connectivity — preferred over alternative solutions.

#### 3. **Scalability & Network Capabilities**
- At least **5 devices** must be able to communicate with each other.
- Devices should maintain connectivity over a distance of approximately **100 meters**.
- Explore integration of **motion sensors** to detect nearby movement (e.g., for triggering measurements or collecting user interaction data).

#### 4. **Real-World Testing & User Feedback**
- Conduct field tests outside the lab environment (urban settings).
- Engage with people on the street to gather opinions on air pollution and user acceptance of the device.

#### 5. **Device Size & Design**
- The current physical size is optimal and **should not be changed**.

### Project Organization & Workflow

- **Weekly Updates:** Every **Monday at 10:00 AM** – short meeting with a brief presentation of progress.
- **Sprint Showcase:** The **last Monday of each month** is a Final Sprint Day with a public showcase of developments.
- **Final Deliverable:** Comprehensive, easy-to-understand documentation enabling anyone to build their own Green Mile Pollution Box.

### Summary of Core Goals

- Improve energy efficiency through battery analysis and hardware optimization  
- Ensure stable internet via SIM cards  
- Enable scalable network of ≥5 devices (100 m spacing)  
- Conduct real-world testing with public interaction  
- Deliver clear, accessible documentation for self-build  
- Maintain current device size – no design changes needed
