namespace ComputeDuck
{
    public class Context
    {
        public Context()
        {
            m_Values = new Dictionary<string, Value>();
            m_UpContext = null;
        }

        public Context(Context upContext)
        {
            m_Values = new Dictionary<string, Value>();
            m_UpContext = upContext;
        }

        public void DefineVariableByName(string name, Value value)
        {
            if (m_Values.ContainsKey(name))
                Utils.Assert("Redefined variable:" + name + " in current context.");
            else
                m_Values[name] = value;
        }

       public void AssignVariableByName(string name, Value value)
        {
            if (m_Values.ContainsKey(name))
                m_Values[name] = value;
            else if (m_UpContext != null)
                m_UpContext.AssignVariableByName(name, value);
            else
                Utils.Assert("Redefined variable:" + name + " in current context.");

        }

        public Value GetVariableByName(string name)
        {
            if (m_Values.ContainsKey(name))
                return m_Values[name];
            if(m_UpContext!=null)
                return m_UpContext.GetVariableByName(name);
            return Value.g_UnknownValue;
        }

        public Dictionary<string, Value> m_Values;
        public Context m_UpContext;
    }
}