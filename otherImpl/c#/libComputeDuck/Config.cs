using System;
using System.Collections.Generic;

namespace ComputeDuck
{
    public class Config
    {
        private static Config? instance = null;
        private string? m_CurExecuteFileDirectory = null;

        public static Config GetInstance()
        {
            if (instance == null)
                instance = new Config();
            return instance;
        }

        public void SetExecuteFileDirectory(string path)
        {
            m_CurExecuteFileDirectory = path;
        }

        public string ToFullPath(string path)
        {
            return m_CurExecuteFileDirectory + path;
        }
    }
}
