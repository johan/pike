//! @xml{<url  href="http://glade.pn.org/">Glade</url>@} is a free GUI builder for
//! GTK+ and Gnome. It's normally used to create C-code, but can also
//! produce code for other languages. @xml{<url 
//! href="http://www.daa.com.au/~james/gnome/">Libglade</url>@} is a utility
//! library that builds the GUI from the Glade XML save files. This
//! module uses libglade and allows you to easily make GUI designs to
//! be used with your Pike applications. Since PiGTK doesn't
//! support Gnome yet, you can only use the normal GTK widgets.
//!
//!

GTK.Widget get_widget( string name );
//! This function is used to get get the widget
//! corresponding to name in the interface description. You would use
//! this if you have to do anything to the widget after loading.
//!
//!

GTK.Widget get_widget_by_long_name( string long_name );
//! This function is used to get the widget
//! corresponding to long_name in the interface description. You would
//! use this if you have to do anything to the widget after
//! loading. This function differs from GladeXML->get_widget, in that
//! you have to give the long form of the widget name, with all its
//! parent widget names, separated by periods.
//!
//!

string get_widget_long_name( GTK.Widget widget );
//! Used to get the long name of a widget that was generated by a
//! GladeXML object.
//!
//!

string get_widget_name( GTK.Widget widget );
//! Used to get the name of a widget that was generated by a
//! GladeXML object.
//!
//!

GTK.GladeXML new( string filename, string|void root, string|void domain );
//! Creates a new GladeXML object (and the corresponding widgets) from
//! the XML file filename. Optionally it will only build the interface
//! from the widget node root. This feature is useful if you only want
//! to build say a toolbar or menu from the XML file, but not the
//! window it is embedded in. Note also that the XML parse tree is
//! cached to speed up creating another GladeXML object for the same
//! file. The third optional argument is used to specify a different
//! translation domain from the default to be used.
//!
//!

GTK.GladeXML new_from_memory( string data, string|void root, string|void domain );
//! Creates a new GladeXML object (and the corresponding widgets) from
//! the string data. Optionally it will only build the interface from
//! the widget node root. This feature is useful if you only want to
//! build say a toolbar or menu from the XML document, but not the
//! window it is embedded in. The third optional argument is used to
//! specify a different translation domain from the default to be used.
//!
//!

GTK.GladeXML signal_autoconnect( mapping callbacks, mixed data );
//! Try to connect functions to all signals in the interface. The
//! mapping should consist of handler name : function pairs. The
//! data argument will be saved and sent as the first argument to all
//! callback functions. 
//!
//!

GTK.GladeXML signal_autoconnect_new( mapping callbacks, mixed data );
//! Try to connect functions to all signals in the interface. The
//! mapping should consist of handler name : function pairs. 
//! This function differs from the signal_connect function in how it
//! calls the callback function.
//!
//! 
//!
//! 
//! The old interface:
//! @pre{
//!   void signal_handler( mixed data, GTK.Object object,
//!                        mixed ... signal_arguments )
//! @}
//! The new interface:
//! @pre{
//!   void signal_handler( mixed ... signal_arguments,
//!                        mixed data, GTK.Object object )
//! @}
//!
//!
