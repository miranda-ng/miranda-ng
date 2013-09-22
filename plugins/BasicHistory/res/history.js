
function toggleFolder(id, imageNode) 
{
    var folder = document.getElementById(id);      
    if (imageNode.previousSibling != null)
    {
		imageNode = imageNode.previousSibling;
    }
    var l = imageNode.src.length;
    if (folder == null) 
    {
    } 
    else if (folder.style.display == "block") 
    {
		if (imageNode != null) 
		{
			if (imageNode.src.substring(l-9,l) == "mnode.ico")
			{
				imageNode.src = imageNode.src.substring(0, l-9) + "pnode.ico";
			}
		}
		folder.style.display = "none";
    } 
    else 
    {
		if (imageNode != null) 
		{
			if (imageNode.src.substring(l-9,l) == "pnode.ico")
			{
				imageNode.src = imageNode.src.substring(0, l-9) + "mnode.ico";
			}
		}
		folder.style.display = "block";
    }
}

var timer;

function ShowMenu(isShow) 
{
	if (timer) 
		clearTimeout(timer);
	
	var menu = document.getElementById("L1");
	if (isShow == 1) 
		menu.style.visibility = "visible";
	else
		menu.style.visibility = "hidden";
} 

function HideMenu() 
{ 
	timer = setTimeout("ShowMenu(0)", 500); 
}

function OpenAll(isOpen) 
{  
	var idMod = 0;
	while(gr = document.getElementById("group" + idMod))
	{
		var imageNode = gr.previousSibling;
		if (imageNode.tagName == null)
			imageNode = imageNode.previousSibling;
		imageNode = imageNode.getElementsByTagName("span")[0].getElementsByTagName("img")[0];
		var l = imageNode.src.length;
		if (gr.style.display == "block")
		{
			if (!isOpen)
			{
				if (imageNode != null) 
				{
					if (imageNode.src.substring(l-9,l) == "mnode.ico")
					{
						imageNode.src = imageNode.src.substring(0, l-9) + "pnode.ico";
					}
				}
				gr.style.display = "none";
			}
		}
		else if (isOpen)
		{
			if (imageNode != null) 
			{
				if (imageNode.src.substring(l-9,l) == "pnode.ico")
				{
					imageNode.src = imageNode.src.substring(0, l-9) + "mnode.ico";
				}
			}
			gr.style.display = "block";
		}
		++idMod;
	}

	ShowMenu(0);
}
