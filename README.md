# Winding Number

Computes the winding number of a test point with respect to a closed polygon.

This project is part of a job interview process, to ease sharing my submisison with the hiring team I've created this public repository. To mitigate the repository showing up in searches I've tried to anonymize the template a bit by removing searchable phrases and resetting git history.

# Algorithm:

My submission computes the winding number of a point with respect to a polygon in two dimensional space by simulating a ray cast up out of the point and searching for edges that cross the ray. When such an edge is found the side of the edge the test point is on is found via taking the z component of the 3d cross product, formed by the three points. The winding count is then modified appropriately.

The prompt requires counting points on the edge of the polygon as within the polygon - something not straight forward to do with taking the cross product alone. For my submission I chose to treat such occurances as "to the left of the edge", this results in changing one inequality expression and adding one extra conditional check.

# IO changes:

The steps I took to address IO stability are in `supplemental/poly_io_notes.txt`
