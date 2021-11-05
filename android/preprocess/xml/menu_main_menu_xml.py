import xml.etree.ElementTree as ET

def _ProcessXML(root):
  # Register namespaces
  ns = { 'android' : 'http://schemas.android.com/apk/res/android' }
  for prefix, uri in ns.items():
    ET.register_namespace(prefix, uri)

  node_str = '<item xmlns:android="http://schemas.android.com/apk/res/android" '\
    'android:id="@+id/open_history_menu_id" ' \
    'android:title="@string/menu_history" ' \
    'android:icon="@drawable/ic_history_googblue_24dp" />'
  node = ET.fromstring(node_str, parser=ET.XMLParser(encoding="utf-8"))

  android_ns = "http://schemas.android.com/apk/res/android"
  parent = root.find('group/[@android:id="@+id/PAGE_MENU"]', namespaces=ns)
  print(ET.tostring(parent))
  child = parent.find('item/[@android:id="@+id/all_bookmarks_menu_id"]', namespaces=ns)
  print(ET.tostring(parent))
  idx = list(parent).index(child)
  parent.insert(idx + 1, node)
  print(ET.tostring(parent))

  return root
  
