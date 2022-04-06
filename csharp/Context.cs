namespace ComputeDuck
{
    public class Context
    {
        public Context()
        {
            m_Values = new Dictionary<string, Object>();
            m_UpContext = null;
        }

        public Context(Context upContext)
        {
            m_Values = new Dictionary<string, Object>();
            m_UpContext = upContext;
        }

        public void DefineVariableByName(string name, Object value)
        {
            if (m_Values.ContainsKey(name))
                Utils.Assert("Redefined variable:" + name + " in current context.");
            else
                m_Values[name] = value;
        }

       public void AssignVariableByName(string name, Object value)
        {
            if (m_Values.ContainsKey(name))
                m_Values[name] = value;
            else if (m_UpContext != null)
                m_UpContext.AssignVariableByName(name, value);
            else
                Utils.Assert("Redefined variable:" + name + " in current context.");

        }

        public Object GetVariableByName(string name)
        {
            if (m_Values.ContainsKey(name))
                return m_Values[name];
            if(m_UpContext!=null)
                return m_UpContext.GetVariableByName(name);
            return null;
        }

        public Dictionary<string, Object> m_Values;
        public Context m_UpContext;
    }
}