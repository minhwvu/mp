import enum
from pathlib import Path
from collections.abc import Iterable


class ModelTags(enum.Enum):
    NA = 0

    ## Modeling features
    linear = 1
    quadratic = 2
    quadraticnonconvex = 3
    socp = 4
    nonlinear = 5
    complementarity = 6
    arc = 7
    plinear = 8         # Requires native handling of pl constraints

    continuous = 10
    integer = 11
    binary = 12         # For name only, AMPL does not distinguish

    logical = 13
    polynomial = 14

    trigonometric = 100
    htrigonometric = 101
    log = 102

    ## Execution mode
    script = 1000

    ## Driver features
    unbdd = 10001
    return_mipgap = 10002
    sens = 10003
    lazy_user_cuts = 10004
    sos = 10005
    presosenc=10006
    funcpieces = 10007

    relax=20006
    warmstart=20007
    mipstart=20008

    multiobj=30009
    obj_priority=30010

    multisol = 40011
    sstatus = 40012              # Basis I/O
    iis = 40013
    iisforce = 40014
    feasrelax = 40100

    check_pl_approx_exp = 50000          # Solvers natively accepting general
    check_pl_approx_log = 50001          # nonlinear constraints.
                                         # The automatic acc:exp etc options allow
                                         # testing own pl approximation

    check_pl2sos2 = 50500                # Solvers accepting PL constraints but wishing
                                         # to test MP's PL -> SOS2 conversion, by acc:pl=1


    ## Solver-specific
    gurobi_cloud = 100000
    gurobi_server=100001


    @staticmethod
    def fromString(l):
        ret = set()
        for s in l:
            ret = ret | {ModelTags[s]}
        return ret


class Model(object):

    """Represents a model"""

    def __init__(self, filename, expsolution, tags, otherFiles=None,
                 overrideName=None, description=None):
        if isinstance(filename, str):
            filename = Path(filename)
        self._filename = filename
        self._expsolution = expsolution
        self._tags = tags
        if otherFiles:
            self._otherFiles = otherFiles if type(
                otherFiles) is list else [otherFiles]
        else:
            self._otherFiles = None
        self._name = overrideName if overrideName else filename.stem
        self._description = description

    def getSolvers(self):
        return self._description["solvers"]

    def hasOptions(self):
        return (self._description is not None) and \
            ("options" in self._description)

    def getOptions(self):
        return self._description["options"]

    def hasExpectedValues(self):
        return (self._description is not None) and \
            ("values" in self._description)

    def getExpectedValues(self):
        return self._description["values"]

    def getExpectedObjective(self):
        return self._expsolution

    def getName(self):
        return self._name

    def isNL(self):
        return self._filename.suffix.lower() == ".nl"

    def getFilePath(self):
        return str(self._filename)

    def getSolFilePath(self):
        return self._filename.with_suffix(".sol")

    def getAdditionalFiles(self):
        return self._otherFiles

    def hasTag(self, tag: ModelTags):
        if isinstance(self._tags, Iterable):
            return tag in self._tags
        else:
            return tag == self._tags

    def hasAnyTag(self, tags: set):
        if tags is None:
            return False
        for tag in tags:
            if self.hasTag(tag):
                return True
        return False

    # Whether this model's tags are a subset of the solver's
    def isSubsetOfTags(self, tags: set):
        if tags is None:
            return False
        for tag in self._tags:
            if tag not in tags:
                return False
        return True

    def isScript(self):
        return self.hasTag(ModelTags.script)

    @staticmethod
    def LINEAR(filename, expsolution,
               vars: ModelTags = ModelTags.continuous):
        return Model(filename, expsolution, [ModelTags.linear, vars])

    @staticmethod
    def QUADRATIC(filename, expsolution,
                  vars: ModelTags = ModelTags.continuous):
        return Model(filename, expsolution, [ModelTags.quadratic, vars])
