-----------------------
MODERN ROW DESIGN GUIDE
-----------------------

New engine logic is very similar to HTML. Each contact rows are 
represented as table. Table devided to rows or columnts. Contact list row
items specified as attribute of some table cells.

1. Tags
-------
There are two tags described cell containers:
<tr> - specify that cell is row.
<tc> - specify that cell is column.
Tags are pair and should be correspondingly closed by </tr> or </tc>
Also allowed short form: <tr ... /> , <tc ... />. 
NOTE: There should be space before close brace: '...[space]/>'.

In general rows can contain only column an vise versa.

2. Attributes
-------------
Each container can contain several attributes:
a)	attributes of cell type if one of following attribute presents 
	appropriate contact list row item will be painted there.

	* avatar - contact's avatar
	* status - contact's status icon
	* time   - contact local time 
	* extra  - contact associated extra images
	* text1, text2, text3 - contact text lines
	
	NOTE: trere are two additional attributes:
	
	* space  - empty space with dinamic width
	* fspace - empty space with fixed width
	
b)	attributes of size (applicable only if space and fspace specified)
    
    * width  - width of space (for dynamic width spaces see bellow)
    * height - height of space
    
    NOTE: value of each attributes should be specified after space
    like:	'width 10  height 5'    

c)  attributes of aligment:
	
	* left, hcenter, right - horizontal aligment (left, center, right)
	* top, vcenter, bottom - vertical aligment (top, center, bottom)
	
	NOTE: default aligment is 'left top'

d)  special attribute

    * layer - meaning that such container is in new layer.

3. Comments and separators
-----------

   All charecters from ';' to the end of line are ignored and can be used
   as comments for template.
   
   Template are case insensitive. Tags and attributes are separated by 
   spaces. Several consiquent spaces are threats as one.
   
   NOTE: there should NOT be spaces beetween 
   *  '<' and 'tr' or 'tc';
   *  '<' and '/' and  'tr' or 'tc'
   *  '/' and '>'
    
4. Rules for use
----------------

-	Whole contact row represents with one of outside tag  <tr> or <tc>, 
    other should be inside this.
    
    e.g.
    <tr>			; this represent whole contact list row and means that
					; inside should be columns
		<tc .. />
		...
		<tc .. />			
    </tr>			; end of parsing here
    
-	Size of parents elements are defined by size of they child as follow:
	
	*	width of 'row' parent is maximum value from sum of width 'columns' 
		inside and maximum width of all layer childs.
	
	*	height of 'column' parent is maximum value from sum of height
		'height' inside and maximum height of all layer childs.	
		
	*   width of 'column' parent and height of 'row' parent are the 
	    maximum of appropriate value  of childs.
	    
-	Minimum size of cells with specified attributes wich described in 
	section 2a is size of appropriate elements (except of width of texts 
	and dynamic width space)

-   Width of dynamic width elements (texts and space) determine as folow:
	
... 
