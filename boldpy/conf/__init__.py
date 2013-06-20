import bold
from boldpy.conf.confimpl import *

_confimpl = PyConf(reportMissing=False).__disown__()
bold.Configurable.setConfImpl(_confimpl)

from boldpy.conf.defparams import *

