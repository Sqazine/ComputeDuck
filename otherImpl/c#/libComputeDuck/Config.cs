using System;
using System.Collections.Generic;

namespace ComputeDuck
{
    public class Config
    {
        private static Config? instance = null;
        private string? m_CurExecuteFilePath = null;

        public static Config GetInstance()
        {
            if (instance == null)
                instance = new Config();
            return instance;
        }

        public void SetExecuteFilePath(string path)
        {
            m_CurExecuteFilePath = path;
        }

        public string ToFullPath(string path)
        {
            return m_CurExecuteFilePath + path;
        }
    }
}
