# Grafana

## Dashboard Variables

The dashboard has a variable `Device` which you can use to toggle between different devices in the
grafana dashboard.

We use a custom query to get the data for a single device filtered by the variable (the name of the
device):

**Example Query: Temperature over time for selected device**

```sql
SELECT s.temperature, s.time_unix AS "time_sec" FROM cmb.SHT41 AS s
INNER JOIN Device AS d ON s.Device_Id = d.id
WHERE d.name = '$Device'
ORDER BY time_unix DESC
```

First we select the data we want to display, then we join the `Device` table on the id of the
device. Now we can filter the data by the device name using the variable `$Device` (`WHERE`).
Finally we order the data by time so that the newest data is shown first.

All dashboards follow this same pattern and only the seleted columns and tables change.

It might be possible to use the device ID instead of the name, but using the name is more user
friendly. If we would use the id you could easily make the query using Grafana's query builder.

Also, it might be possible to achieve the same using Grafana's query builder and filter /
transformation options, but we couldn't figure it out.
