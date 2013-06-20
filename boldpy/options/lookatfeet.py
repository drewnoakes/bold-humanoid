import bold

class LookAtFeet(bold.Option):
    def __init__(self, id):
        super(LookAtFeet, self).__init__(id)

        self.feetX = self.getParamDbl("feetX", 1.0)
        self.feetY = self.getParamDbl("feetY", 1.0)

    def runPolicy(self):
        return []

