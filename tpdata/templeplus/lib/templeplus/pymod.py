import tpdp

class PythonModifier(tpdp.ModifierSpec):
    def AddHook(self, eventType, eventKey, callShit, argsTuple ):
        self.add_hook(eventType, eventKey, callShit, argsTuple)