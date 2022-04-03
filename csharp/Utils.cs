namespace ComputeDuck
{
    public class Utils
    {
        public static void Assert(string msg)
        {
            Console.WriteLine(msg);
            System.Environment.Exit(1);
        }

        public static string ReadFile(string path)
        {
            if (!File.Exists(path))
                Assert("Failed to open file:" + path);
            return System.IO.File.ReadAllText(path);
        }
    }
}