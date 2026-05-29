# Project Overview

## Goal
Get the prototypes out in the field with reliable performance and visible system health, including battery status.


## Roadmap

### Sprint 2
- **Objective:** Build stable platform and infrastructure  
- **Output:** GitLab, MQTT, dashboard, PCB, working prototype  
- **Status:** In Progress  

### Sprint 3–4
- **Objective:** Deliver public DIY kit and field-ready system  
- **Output:** Kit, enclosure, documentation, deployed prototypes  
- **Status:** Planned  


## Sprint 2 — Development Environment

### Objective
Build a stable, documented platform based on a reliably working sensor and prepare it for real-world deployment, including initial battery monitoring.

### Embedded
- Get the prototype running reliably on battery  
- Implement deep sleep functionality in the prototype firmware  
- Add initial battery status monitoring  
- Ensure Grafana is fully working with live data  
- Update and clean up prototype documentation  

### GitLab & CI Setup
- Set up CI pipeline  
- Reference or migrate relevant earlier work  

### Data Pipeline (MQTT)

**Architecture:**  
Sensor → MQTT Broker → Storage → Consumers  

**Tasks:**
- Deploy MQTT broker  
- Build service  
- Document full data flow with a diagram  

### Custom PCB Design
- Design integrated PCB  
- Review with team  
- Order prototype  

### Documentation
- Update prototype documentation continuously  
- Ensure all configurations, schemas, and system diagrams are stored in the repository  
- Prepare documentation to evolve into DIY kit guides in later sprints  


## Sprint 2 Deliverables
- GitLab repository with CI pipeline and documented workflow  
- MQTT broker running with defined JSON schema  
- Working Grafana dashboard with live data  
- Prototype running reliably on battery with deep sleep implemented  
- Initial battery status visibility  
- Updated and complete prototype documentation  
- PCB prototype ordered  


## Sprint 2 Summary
**Build and stabilize the system**
- Infrastructure (GitLab, MQTT, database, Grafana)  
- Reliable battery-powered prototype with deep sleep  
- Baseline battery monitoring  
- Documentation and coordination  
- Preparation for scaling into a DIY kit in Sprint 3–4  


## Upcoming Work (Next Week)
- Carry out a sanity check on current battery usage  
- Deploy prototypes in the field  
- Add a resistor on the board to ensure consistent data flow  
- Finalize documentation so that UvA students can understand the system, how it works, and how to connect to endpoints  


## Sprint 4 Goals
- Implement the new 3D model design for the prototype  
- Replicate the existing working boards and create new ones for field testing  
- Implement a battery check so that the latest battery status is visible in Grafana  
- Test the PCB created for the Green Mile sensor box with all sensors  