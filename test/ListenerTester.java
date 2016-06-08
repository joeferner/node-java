public class ListenerTester {
    private ListenerInterface _listener;

    public void setListener(ListenerInterface listener) {
        this._listener = listener;
    }

    public void raiseEvent() {
        if(this._listener == null)
            return;

        java.util.ArrayList<String> list = new java.util.ArrayList<String>();
        list.add("hello");
        list.add("from");
        list.add("Java");
        
        this._listener.onEvent(list, java.lang.Runtime.getRuntime());
    }
}