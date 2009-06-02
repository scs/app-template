Array.prototype.joinLines = function () {
	var res = this.join("\n")
	
	if (res)
		res += "\n";
	
	return res;
}

String.prototype.splitLines = function () {
	var list = this.split("\n");
	
	if (list.length > 0 && !list[list.length - 1])
		list.pop();
	
	return list;
}

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

// Removes duplicate elements from an array.
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

// getInputNames: Get a list of names of all input elements in the given context. context is optional and defaults to the document.
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
	
	return list.joinLines();
}

function parseValues(data) {
	var obj = { };
	
	$.each(data.splitLines("\n"), function () {
		var pos = this.indexOf(":");
		
		obj[$.trim(this.slice(0, pos))] = $.trim(this.slice(pos + 1));
	})
	
	return obj;
}

function exchangeState(data, onLoad, onError) {
	$.ajax({
		async: true,
		cache: false,
		contentType: "text/plain",
		data: serializeValues(data),
		error: onError,
		success: function (data) {
			onLoad(parseValues(data));
		},
		timeout: 2000,
		type: "POST",
		url: "/cgi-bin/cgi"
	});
}

function asynLoadImage(url, onLoad, onError) {
	var img = $(new Image());
	
	img.load(onLoad);
	img.error(onError);
	img.attr("src", url /*+ "?dummy=" + (new Date()).getTime()*/);
}

var offBanner = {
	active: false,
	show: function () {
		var wheelPos = 0;
		
		if (this.active)
			return;
		
		this.active = true;
		$("#off").stop(true, true).fadeIn("fast");
		
		$("#off .wheel").everyTime("100ms", function () {
			wheelPos = (wheelPos + 1) % 12;
			$(this).css("background-position", -wheelPos * 32 + "px");
		});
	},
	hide: function () {
		if (!this.active)
			return;
		
		this.active = false;
		$("#off").stop(true, true).fadeOut("fast", function () {
			$(".wheel", this).stopTime();
		});
	}
}

var stateControl = {
	currentState: "online",
	pulledState: "",
	pullState: function (state) {
		var that = this;
		
		if (state != this.pulledState) {
			$(document).stopTime("stateControl");
			this.pulledState = state;
		}
		
		if (state != this.currentState) {
			$(document).oneTime("2s", "stateControl", function () {
				that.changeState(state);
			});
		}
	},
	changeState: function (state) {
		var states = {
			offline: function () {
				offBanner.show();
			},
			online: function () {
				offBanner.hide();
			}
		};
		
		this.currentState = state;
		states[state]();
	}
}

function updateCycle() {
	function offline() {
		stateControl.pullState("offline");
		
		$(document).oneTime("0.5s", function () {
			exchangeState({ }, online, offline);
		});
	}
	
	function online() {
		stateControl.pullState("online");
		
		exchangeState(getInputValues($("#options-box")), function (data) {
			asynLoadImage("image.bmp?" + data.imgTS, function () {
				$(this).attr("id", "image");
				$("#image").replaceWith(this);
				
				// Close the loop.
				online();
			}, function (event) {
			//	console.log(event);
				offline();
			});
		}, function (request, status) {
		//	console.log(status);
			offline();
		});
	}
	
	online();
}
