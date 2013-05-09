%{
#include <Agent/agent.hh>
%}

%include "agent.hh"

namespace bold
{

  %extend Agent {
  public:
    void onThinkEndConnect(PyObject* pyFunc)
    {
      $self->onThinkEnd.connect(
        [pyFunc]() {
          PyEval_CallObject(pyFunc, Py_BuildValue("()"));
        });
    }
  };
}
