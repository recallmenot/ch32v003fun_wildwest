# This script generates and populates some lookup-tables for HiVoPuCounter.c

import numpy as np


file_path = 'hivopucounter.c'
search_sting_bins_V = 'const uint16_t bins_V[] = {'
search_sting_bins_adc = 'const uint16_t bins_adc[] = {'
search_sting_addr = 'const uint16_t addr[] = {'


voltage_max_V = 3200 # the maximal voltage the logger can log in [V]
voltage_min_V = 420 # the minimal voltage the logger can log in [V]
number_bins = 100 # the number of bins that the logger should differentiate


adc_value_Vmax = 1023
adc_value_Vmin = voltage_min_V / voltage_max_V * adc_value_Vmax


with open(file_path, 'r') as file:
    content = file.readlines()

for index, element in enumerate(content):
    if search_sting_bins_V in element:
        index_bins_V = index

for index, element in enumerate(content):
    if search_sting_bins_adc in element:
        index_bins_adc = index

for index, element in enumerate(content):
    if search_sting_addr in element:
        index_addr = index


bins_Vfloat = np.linspace(voltage_min_V, voltage_max_V, number_bins).tolist()

bins_Vint = []

for i in bins_Vfloat:
    bins_Vint.append(int(i))

bins_adcval = []

for i in bins_Vfloat:
    bins_adcval.append(int(i / voltage_max_V * adc_value_Vmax))

addr = [i * 4 for i in range(number_bins)]



string_bins_Vint = ','.join(map(str, bins_Vint))
string_adcval = ','.join(map(str, bins_adcval))
string_addr = ','.join(map(str, addr))
replacement_string_Vint = search_sting_bins_V + string_bins_Vint + '};\n'
replacement_string_adcval = search_sting_bins_adc + string_adcval + '};\n'
replacement_string_addr = search_sting_addr + string_addr + '};\n'

content[index_bins_V] = replacement_string_Vint
content[index_bins_adc] = replacement_string_adcval
content[index_addr] = replacement_string_addr

with open(file_path, 'w') as file:
    for item in content:
        file.write(f"{item}")


















