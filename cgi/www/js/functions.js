function getSearchArgs() {
	var parts = document.location.search.slice(1).split("&");
	var result = { };
	
	$.each(parts, function () {
		var pair = this.split("=");
		
		if (pair.length == 1)
			result[pair[0]] = true;
		else
			result[pair[0]] = pair.slice(1).join("=");
	})
	
	return result;
}

function setLanguage(lang) {
	$("*[lang]").hide();
	$("*[lang|=" + lang + "]").show();
	$("#lang-select a").removeClass("active");
	$("#lang-select *[id|=" + lang + "]").addClass("active");
}

function createElement(tag, attrs, contents) {
	var elem = $(document.createElement(tag));
	
	if (attrs)
		$.each(attrs, function (key, value) {
			elem.attr(attrs);
		});
	
	if (contents)
		$.each(contents, function (key, value) {
			elem.append(value);
		});
	
	return elem;
}

function buildControls() {
	$("input([type=checkbox], [type=radio]):parent").each(function () {
		var id = $(this).attr("name") + "+" + $(this).attr("value");
		
		$(this).attr("id", id)
		$(this).after(createElement("label", { "for" : id }, $(this).contents()));
		$(this).empty();
	})
}

function removeDuplicates(arr) {
	var newArr = [];
	var arr = $.makeArray(arr);
	
	outer: for (var i in arr) {
		for (var j in newArr)
			if (newArr[j] == arr[i]) 
				continue outer;
		
		newArr.push(arr[i]);
	}
	
	return newArr;
}

/* getInputNames: Get a list of names of all input elements in the given context. context is optional and defaults to the document. */
function getInputNames(context) {
	var foo = $("input", context);
	
	return removeDuplicates($("input", context).map(function() {
		return $(this).attr("name");
	}))
}

function getInputValue(name, context) {
	var elem = $("input[name=" + name + "]", context);
	var type = elem.attr("type");
	
	if (type == "checkbox")
		if (elem.length == 1)
			return (elem.fieldValue().length != 0).toString();
		else
			return elem.fieldValue().join(" ");
	else if (type == "radio")
		return elem.fieldValue().join(" ");
	else
		return elem.fieldValue()[0];
}

/* getInputValues: Get a map of input element names to their values. context is optional and defaults to the document. */
function getInputValues(context) {
	var table = { };
	
	$.each(getInputNames(context), function () {
		table[this] = getInputValue(this, context);
	});
	
	return table;
}

function serializeValues(data) {
	var list = [];
	
	$.each(data, function (key) {
		list.push(key + ": " + this);
	})
	
	return list.join("\n");
}

function parseValues(data) {
	var obj = { };
	
	$.each(data.split("\n"), function () {
		var pos = this.indexOf(":");
		
		obj[$.trim(this.slice(0, pos))] = $.trim(this.slice(pos + 1));
	})
	
	return obj;
}

function exchangeState(data, success, failure) {
	$.ajax({
		async: true,
		cache: false,
		contentType: "text/plain",
		data: serializeValues(data),
		error: failure,
		success: function (data) {
			success(parseValues(data));
		},
		timeout: 5000,
		type: "POST",
	//	url: "/cgi-bin/cgi"
		url: "http://localhost/~michi/cgi-bin/cat"
	});
}

function loadImage(url) {
	var img = new Image();
	
	img.src = url + "?dummy=" + (new Date()).getTime();
	
	return img;
};
