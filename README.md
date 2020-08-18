# Channel-Sounder (work in progress)

This channel-sounder uses [UHD](https://github.com/EttusResearch/uhd) and one or two [USRP N310](https://kb.ettus.com/N300/N310)/[USRP N321](https://kb.ettus.com/N320/N321) to measure the RF channel. We transmit sequences with low cross-correlation properties at the transmitter, record samples at the receiver over a fixed period of time and save the data in binary files. We then process the data offline in MATLAB.

So far we can either build a 2x2 system at 200MS/s (one USRP N321) or we can build a 4x4 system at 125MS/s (two USRP N321). A 4x4 system at 200MS/s is not stable yet as we experience occasional under- and/or overruns. With two USRP N310 we could theoretically build an 8x8 system at 50MS/s.

## What does it look like?
[Pic0](https://user-images.githubusercontent.com/20499620/90509459-a54f5480-e159-11ea-8260-f379ae993ea6.jpg)
[Pic1](https://user-images.githubusercontent.com/20499620/90509487-ad0ef900-e159-11ea-98e9-1f6a9abc1624.jpg)
![eye_catcher](https://user-images.githubusercontent.com/20499620/90509434-9a94bf80-e159-11ea-89e9-5c7321482b61.jpg)

## How to use
```bash
git clone https://github.com/maxpenner/channel-sounder.git
cd channel-sounder
mkdir build
mkdir data
cd build
cmake ../
make
```
Each USRP has three IP addresses, one management address and two 10-Gigabit ports. When using DPDK:
```bash
sudo su
```
To record with a single USRP (2x2 @ 200MS/s):
```bash
./channelsounder --args "type=n3xx,mgmt_addr=192.168.1.156,addr=192.168.10.2,second_addr=192.168.20.2,master_clock_rate=200e6,use_dpdk=1" --channels "0,1" --rx_rate 200e6 --rx_subdev "A:0 B:0" --tx_rate 200e6 --tx_subdev "A:0 B:0" --duration 10
```

To record with two USRPs (4x4 @ 125MS/s):
```bash
./channelsounder --args "type0=n3xx,mgmt_addr0=192.168.1.156,addr0=192.168.10.2,second_addr0=192.168.20.2,time_source0=internal,clock_source0=external,master_clock_rate0=250e6,type1=n3xx,mgmt_addr1=192.168.1.157,addr1=192.168.30.2,second_addr1=192.168.40.2,time_source1=external,clock_source1=external,master_clock_rate1=250e6,use_dpdk=1" --channels "0,1,2,3" --rx_rate 125e6 --rx_subdev "A:0 B:0" --rx_delay 1.0 --tx_rate 125e6 --tx_subdev "A:0 B:0" --tx_delay 1.0 --duration 10
```

More examples can be found in utils/uhd_record_instructions.

## Folders
- **data/**: target folder for binary data
- **pics/**: pictures of testbed (not a part of the repository)
- **process/**: MATLAB code for offline processing of binary data
- **record/**: C++ code for recording binary data
- **utils/**: files with UHD-specific instructions for recording data and some scripts to setup testbed

## Hard- and Software
- **USRP N321**: 2TX + 2RX ports, each capable of sampling at 200MS/s
- **USRP N310**: 4TX + 4RX ports, each capable of sampling at 100MS/s
- **Stanford Research Systems FS725**: 10MHz source, PPS is provided by one of the USRPs
- **Intel i9 10980XE**: 18 core cpu
- **Intel 82599ES 10-Gigabit SFI/SFP+**: this NIC has two 10-Gigabit ports, I use two cards, so four 10-Gigabit ports in total ([Pic1](https://user-images.githubusercontent.com/20499620/90509487-ad0ef900-e159-11ea-98e9-1f6a9abc1624.jpg))
- **UHD-3.15.LTS**
- **Ubuntu 18.04.2 LTS**

To avoid under- and overruns:
- use [UHD with DPDK](https://kb.ettus.com/Getting_Started_with_DPDK_and_UHD)
- use low-latency kernel
- disable Hyper-threading
- disable frequency scaling, sleep states etc. ([HowTo0](
https://kb.ettus.com/USRP_Host_Performance_Tuning_Tips_and_Tricks), [HowTo1](https://gitlab.eurecom.fr/oai/openairinterface5g/-/wikis/OpenAirKernelMainSetup#power-management))

## To Do
- stabilize system with 4x4 at 200MS/s and 8x8 at 50MS/s
- optimize transmitted sequences and add to C++ code
- finish MATLAB offline processing
- finish MATLAB simulation with [Rayleigh MIMO channel](https://de.mathworks.com/help/comm/ref/comm.mimochannel-system-object.html)
- write optimization depending of delay- and Doppler spread of RF channel
