var g = new JustGage({
    id: "gauge",
    value: 67,
    min: 0,
    max: 100,
    title: "Oil Pressure"
});
var func = function(newval) {
    //    dataDiv = document.getElementById('currentData');
    //    dataDiv.innerHTML = newval;
    g.refresh(newval);
};
window.addEventListener('load', 
  function() { 
    func(0);
  }, false);
