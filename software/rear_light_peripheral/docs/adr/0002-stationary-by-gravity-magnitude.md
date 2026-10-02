# Stationary is detected by acceleration magnitude near 1g

Smart auto-off counts consecutive samples whose acceleration magnitude is within a tolerance band around 1g, instead of measuring the absence of change. A bike at constant speed on perfectly smooth road would also read 1g, but real roads are uneven enough that a moving bike regularly leaves the band, which resets the count. Detecting "no variation" was the alternative and was rejected. Do not "fix" this to a variance measure without checking how long a real ride stays inside the band.
