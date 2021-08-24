![Bold Hearts Logo](bold-hearts-logo-640x289.png)

# bold-humanoid

The code describes a fully autonomous agent that participates in the kid-size Humanoid Soccer League of the international RoboCup Federation.

It was developed at the University of Hertfordshire who has a long tradition of participating in RoboCup with the team _Bold Hearts_. Originally competing in the 3D simulation league until 2013 when the university acquired five DARwIn-OP robots. In late 2013 I took a research position at the university to help develop this platform for the 2014 World Cup.

The _bold-humanoid_ platform includes:

- Custom computer vision pipeline, running at 30Hz (every 33ms)
- Hardware control, running at 125Hz (every 8ms)
- Environment and object detection
- Gait generation for locomotion
- Localisation
- Gaze control
- Motion planning, including obstacle avoidance and team mate coordination
- Orientation and motion estimation via IMU (gyro and accelerometer)
- Behaviour planning
- Team communication and strategy
- Browser-based diagnostic and configuration tools for rapid development (Round Table)

The code was designed to run on the [Robotis DARwIn-OP](https://en.wikipedia.org/wiki/DARwIn-OP) robotic platform. The onboard computer is a low-spec Intel Atom CPU, running a custom Ubuntu Linux installation that supports the `boldhumanoid` executable.

This repo contains the code as it stood after the RoboCup World Cup in Brazil 2014.

This video contains footage from the 2014 World Cup tournament between 24 teams from around the world, in which the Bold Hearts placed second against the legendary Japanese team CIT Brains:

[![BoldHearts robotics footage from the 2014 RoboCup World Cup in Brazil](https://img.youtube.com/vi/pzYHAp7b7sY/0.jpg)](https://www.youtube.com/watch?v=pzYHAp7b7sY)

---

See the [wiki](https://github.com/drewnoakes/bold-humanoid/wiki) for more details.

The Bold Hearts team continues development of this project [on GitLab](https://gitlab.com/boldhearts/bold-humanoid).

All code published under the APACHE-2.0 license.
