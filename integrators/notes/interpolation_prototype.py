class Series:
    
    class CriticalPointList:
        """  List of times where the function isn't smooth enough """
        def __init__(self, series, criticalPoints):
            self.series = series
            self.critPoints = criticalPoints

        def __call__(self, index):
            """ Return the critical point with the given index.
                -1 indicates initial value (no critical point below) while -2 (or a value higher than the number of points) indicates final value (no critical point above).
            """
            self.nPoints = len(self.critPoints) # Move somewhere where we don't do this everytime

            assert(index >= -2)
            if index == -1:
                return self.series.series_t[0]
            elif index == -2 or index >= self.nPoints:
                return self.series.series_t[-1]
            else:
                return self.critPoints[index]


    def __init__(self, ip):
        """ f here is the _derivative 
            series_t, series_x should be added externally
        """
        self.ip = ip
        self.v = -1
        self.coeff = np.zeros(ip)
        self.d = np.zeros(ip - 1)
        self.series_t = []
        self.critPoints = self.CriticalPointList(self, [])  # List of times where the function isn't smooth enough
        self.neighbourCritPointIdxs = [-1,-1] # Index of nearest critical points that bound the interpolated point from below (idx: 0) and above (idx: 1). -1 indicates initial value (no critical point below) while -2 indicates final value (no critical point above).
        #self.series_t = []
        
    def interpolate(self, t, accelerate=True):
        assert((t >= self.series_t[0]) and (t <= self.series_t[-1])) # Ensure we are interpolating within bounds

        if t in self.series_t:
            # Don't interpolate if we are at a node
            return self.series_x[self.series_t.index(t)]

        v = self.getV(t)
        if v != self.v:   # TODO Is there any way to not have this and still have incorrect coefficients ?
            if (v == self.v + 1 and accelerate):
                self.v = v
                self.getNextHermiteCoefficients()
            else:
                self.v = v
                self.getHermiteCoefficients()
            #print("Computing coefficients for v = " + str(v) + "...")
        return self.computePoly(t)

    def getV(self, t):
        # NOTE: This function assumes that the stepsize is sufficiently small to allow at least
        #       ip steps between any two critical points.
        #       It also does not choose proper points for a decreasing increment
        #       (can choose points such that all but the first are on same side 
        #       of interpolation point)

        self.npoints = len(self.series_t)
        m = len(self.series_t) - 1   # Maximum value to which we have integrated
        v = self.v
        xi= self.getNeighbourCritPoints(t)

        if self.series_t[v - self.ip + 1] > t:
            #print("yo: " + str(t) + " (v=" + str(v) + ")")
            v = -1  # v is already too high
            # TODO: make 'reverse' function for this case, instead of just restarting ?

        if v == -1:
            v = self.ip - 1 # Ensure that interpolation does not try to count points before the first
#            v = self.ip  # Ensure that interpolation starts no earlier than second step
            
        while self.series_t[v] < xi[0]:
            v += 1

        l = floor(self.ip/2)
        while (self.series_t[v - l] <= t and v < m and self.series_t[v] < xi[1]):
             v += 1
        
        return v


    def getNeighbourCritPoints(self, t):

        if self.critPoints(self.neighbourCritPointIdxs[1]) < t :
            while (self.critPoints(self.neighbourCritPointIdxs[1]) < t) :
                self.neighbourCritPointIdxs[1] += 1
        
            self.neighbourCritPointIdxs[0] = self.neighbourCritPointIdxs[1] - 1

        elif self.critPoints(self.neighbourCritPointIdxs[0]) > t :
            while (self.critPoints(self.neighbourCritPointIdxs[0]) > t) :
                self.neighbourCritPointIdxs[0] -= 1
        
            self.neighbourCritPointIdxs[1] = self.neighbourCritPointIdxs[0] + 1

        return (self.critPoints(self.neighbourCritPointIdxs[0]),
                self.critPoints(self.neighbourCritPointIdxs[1]))

        
    def getHermiteCoefficients(self):
        v = self.v
        series_t = self.series_t
        series_x = self.series_x
        self.coeff[0] = series_x[v]

        assert(v - self.ip + 1 >= 0)  # must have at least ip points behind v to interpolate with
        for i in range(0, self.ip - 1):
            self.d[i] = (series_x[v - i - 1] - series_x[v - i])/(series_t[v - i - 1] - series_t[v - i])
#            if i == 0:
#                print(series_x[v - i - 1], series_x[v - i], series_t[v - i - 1], series_t[v - i])

        self.coeff[1] = self.d[0]


        for n in range(2, self.ip):
            for i in range(0,  self.ip - n):
#                if i == 0:
#                    print(self.d[i+1], self.d[i], self.series_t[v - i - n], self.series_t[v - i])

                self.d[i] = (self.d[i+1] - self.d[i])/(series_t[v - i - n] - series_t[v - i])
#            print("d = " + str(self.d))
            self.coeff[n] = self.d[0]

    def getNextHermiteCoefficients(self):
        v = self.v

#        print("coeff: " + str(self.coeff))

        a = self.coeff[0]
        self.coeff[0] = self.series_x[v]
        
        for i in range(1, self.ip - 1):
            a1 = self.coeff[i]    # Might be more efficient to just copy the whole coeff array once in one go
            self.coeff[i] = (a - self.coeff[i-1])/(self.series_t[v - i] - self.series_t[v])
            a = a1

#        print(a, self.coeff[self.ip-2], self.series_t[v - self.ip + 1], self.series_t[v])
        self.coeff[self.ip - 1] = (a - self.coeff[self.ip-2])/(self.series_t[v - self.ip + 1] - self.series_t[v])
        
    def computePoly(self, t):
        coeff = self.coeff
        series_t = self.series_t
        v = self.v
        
        b = coeff[self.ip - 1]
        for i in range(0, self.ip - 1):
            b = (t - series_t[v - self.ip + 2 + i])*b + coeff[self.ip - 2 - i]

        return b
