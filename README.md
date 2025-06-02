# SCA_VEGA
### Created by Team "RTL Rangers"

----------------------------------------------------
Main file is **[implementation.c](https://github.com/Arjun-0017/SCA_VEGA/blob/main/implementation.c)**  
Dependencies files are **[aes.c](https://github.com/Arjun-0017/SCA_VEGA/blob/main/aes.c)** and **[aes.h](https://github.com/Arjun-0017/SCA_VEGA/blob/main/aes.h)**  

-------------------------------------------------------
The **[implementation.c](https://github.com/Arjun-0017/SCA_VEGA/blob/main/implementation.c)** code reads data from **[Power_Trace_Data.csv](https://github.com/Arjun-0017/SCA_VEGA/blob/main/Power_Trace_Data.csv)** file and then processes the data.  

### Generated output
+ Mean Power (Mean)
+ Peak Power (Peak)
+ Total Energy (Energy)
+ Hamming Distance (HammingDist)

-----------------------------------------------------
## Working flow of implementation.c code
Functions
+ hamming_distance
+ extract_features
+ load_data_from_csv
+ float_to_fixed_bin
+ main
--------------------------------
### load_data_from_csv
This function reads data from **Power_Trace_Data.csv** and stores the data plaintexts, ciphertexts, keys, power traces.  

### hamming_distance
This function reads the stored data **ciphertexts** and internally computes hamming distance.  

### extract_features
This functin loads the power trace data (floating point data).  
And calculates Mean Power, Peak Power, Total Energy  (All data calculated in floating point format)  

### float_to_fixed_bin
This function represents the calculated floating point data into fixed point representation.  
Here we've used Qm.n format, where m=2, n=10, i.e Q2.10 format.  

---------------------------------------------
|     Field         | Floating |  Fixed     |
|-------------------|----------|------------|
| Mean Power        | mean     | mean_bin   |
| Peak Power        | peak     | peak_bin   |
| Total energy      | energy   | enregy_bin |
| Hamming Distance  | h_dist   | hd_bin     |
---------------------------------------------




