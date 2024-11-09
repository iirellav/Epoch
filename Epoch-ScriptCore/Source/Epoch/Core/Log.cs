namespace Epoch
{
    public static class Log
    {
        internal enum LogLevel
        {
            Debug   = 1 << 0,
            Info    = 1 << 1,
            Warn    = 1 << 2,
            Error   = 1 << 3,
        }

        public static void Debug(string aFormat, params object[] aParameters) => InternalCalls.Log_LogMessage(LogLevel.Debug, FormatUtils.Format(aFormat, aParameters));
        public static void Info(string aFormat, params object[] aParameters) => InternalCalls.Log_LogMessage(LogLevel.Info, FormatUtils.Format(aFormat, aParameters));
        public static void Warn(string aFormat, params object[] aParameters) => InternalCalls.Log_LogMessage(LogLevel.Warn, FormatUtils.Format(aFormat, aParameters));
        public static void Error(string aFormat, params object[] aParameters) => InternalCalls.Log_LogMessage(LogLevel.Error, FormatUtils.Format(aFormat, aParameters));

        public static void Debug(object aValue) => InternalCalls.Log_LogMessage(LogLevel.Debug, FormatUtils.Format(aValue));
        public static void Info(object aValue) => InternalCalls.Log_LogMessage(LogLevel.Info, FormatUtils.Format(aValue));
        public static void Warn(object aValue) => InternalCalls.Log_LogMessage(LogLevel.Warn, FormatUtils.Format(aValue));
        public static void Error(object aValue) => InternalCalls.Log_LogMessage(LogLevel.Error, FormatUtils.Format(aValue));
    }
}
