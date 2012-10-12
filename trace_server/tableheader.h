
class MetaHeaderView(QtGui.QHeaderView):

def __init__(self,orientation,parent=None):
super(MetaHeaderView, self).__init__(orientation,parent)
self.setMovable(True)
self.setClickable(True)
# This block sets up the edit line by making setting the parent
# to the Headers Viewport.
self.line = QtGui.QLineEdit(parent=self.viewport()) #Create
self.line.setAlignment(QtCore.Qt.AlignTop) # Set the Alignmnet
self.line.setHidden(True) # Hide it till its needed
# This is needed because I am having a werid issue that I believe has
# to do with it losing focus after editing is done.
self.line.blockSignals(True)
self.sectionedit = 0
# Connects to double click
self.sectionDoubleClicked.connect(self.editHeader)
self.line.editingFinished.connect(self.doneEditing )

def doneEditing(self):
# This block signals needs to happen first otherwise I have lose focus
# problems again when there are no rows
self.line.blockSignals(True)
self.line.setHidden(True)
oldname = self.model().dataset.field(self.sectionedit)
newname = str(self.line.text())
self.model().dataset.changeFieldName(oldname, newname)
self.line.setText('')
self.setCurrentIndex(QtCore.QModelIndex())

def editHeader(self,section):
# This block sets up the geometry for the line edit
edit_geometry = self.line.geometry()
edit_geometry.setWidth(self.sectionSize(section))
edit_geometry.moveLeft(self.sectionViewportPositio n(section))
self.line.setGeometry(edit_geometry)

self.line.setText(self.model().dataset.field(secti on).name)
self.line.setHidden(False) # Make it visiable
self.line.blockSignals(False) # Let it send signals
self.line.setFocus()
self.line.selectAll()
self.sectionedit = section
