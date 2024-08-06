# Road to Recovery

Road to Recovery is a project designed to facilitate seamless video and audio transmission between a Medic located at a hospital and a Paramedic in an ambulance. The Paramedic wears a VR headset with passthrough enabled, which is mirrored onto a laptop using `scrcpy` and then transmitted over VNC to the Medic. This allows the Medic to see the live feed and guide the Paramedic through necessary medical procedures.

## System Architecture
The following diagram represents the system architecture followed throughout the project:
![Road to Recovery](docs/r2r_system_diagram.png)

## Main Modules
The following diagrams represents the main modules that were used by the system:
- **AR App** ![AR App](docs/ar_app.png)
- **Remote App (Ambulance)** ![Remote App (Ambulance)](docs/remote_app_ambulance.png)
- **Remote App (Hospital)** ![Remote App (Hospital)](docs/remote_app_hospital.png)

## Features

- **Real-time Video and Audio Transmission:** Live video feed from the Paramedic’s VR headset to the Medic.
- **VR Headset Passthrough:** Allows the Paramedic to see their surroundings while transmitting the view.
- **scrcpy Integration:** Mirrors the VR headset feed onto a laptop.
- **VNC Transmission:** Transmits the mirrored feed from the laptop to the Medic's client.
- **Two-way Communication:** Enables the Medic to provide real-time instructions to the Paramedic.

## System Requirements

### Paramedic Side (Ambulance)
- VR Headset with Passthrough capability
- Android Device
- Laptop with:
  - Windows

### Medic Side (Hospital)
- Computer with:
  - Windows

## Installation

### Paramedic Side

1. **Clone the repository**:
    - The main components needed are:
        - VNC Server App
        - scrcpy
        - VoiceChat
        - start SERVER.bat


### Medic Side (Hospital)

1. **Clone the repository**:
    - The main components needed are:
        - VNC Client App
        - VoiceChat
        - start CLIENT.bat

## Usage

1. **Medic**:
   - Run the 'start CLIENT.bat' file.

2. **Paramedic**:
   - Run the 'start SERVER.bat' file.

## Troubleshooting

- **No Video Feed**: Ensure the VR headset is properly connected and passthrough mode is enabled. Check the USB connection between the Android device and the laptop.
- **VNC Connection Issues**: Verify that the VNC server is running on the Paramedic's laptop and that the network settings allow for VNC connections.
- **Audio Problems**: Ensure that the audio settings on both ends are configured correctly and that the necessary permissions are granted for audio transmission.

## Contributing

We welcome contributions to enhance the functionality and reliability of Road to Recovery. Please fork the repository and create a pull request with your improvements.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgements

- Thanks to the developers of `scrcpy` for providing a powerful and flexible screen mirroring solution.
- Thanks to the VNC community for their robust and reliable remote desktop solutions.

## Contact

For any questions or feedback, please contact us at [tiago.ribeiro@logimade.com].

