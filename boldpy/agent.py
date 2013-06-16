import bold

_agent = None

def createAgent():
    global _agent
    _agent = bold.Agent()

def getAgent():
    global _agent
    if (not _agent):
        createAgent()
    return _agent

