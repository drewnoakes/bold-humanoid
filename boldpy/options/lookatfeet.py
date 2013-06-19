import bold

class LookAtFeet(bold.Option):
    def __init__(self, id):
        super(LookAtFeet, self).__init__(id)
        x = self.getParamDbl("feetX", 1.0)
        print("c " + str(x))
        bla = self.getParamDbl("feetY", -67.5)
        print("c " + str(x) + " " + str(bla))

    def runPolicy(self):
        return []

