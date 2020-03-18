from geographiclib.geodesic import Geodesic

res = Geodesic.WGS84.Inverse(40.6, -73.8, 51.6, -0.5)
print(res)
