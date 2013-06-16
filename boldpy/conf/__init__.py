import bold
from boldpy.conf.confimpl import *

bold.Configurable.setConfImpl(PyConf(reportMissing=True).__disown__())

from boldpy.conf.defparams import *

